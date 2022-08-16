#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "scaler_defs.h"

volatile uint32_t *mapped;


bool save_log = false;
char log_file_name[32];
enum LogLevel log_level = kWarning;
FILE *log_file = NULL;

void Log(const char *info, enum LogLevel level) {
	if (!save_log || level > log_level) {
		return;
	}
	if (!log_file) {
		log_file = fopen(log_file_name, "w");
		if (!log_file) {
			fprintf(stderr, "Error: Could not open log file %s.\n", log_file_name);
			exit(-1);
		}
	}
	time_t now_time = time(NULL);
	fprintf(log_file, "%s[%c%s] %s\n", ctime(&now_time), log_level_name[level][0]-32, log_level_name[level]+1, info);
	fflush(log_file);
	return;
}


struct ScalerData scaler_array[kMaxScalerArraySize];
uint32_t scaler_array_tail = 0;
uint32_t scaler_array_head = 0;
uint32_t scaler_array_size = 0;

pthread_mutex_t scaler_array_mutex;


uint32_t FillEmptyScalerResponse(char *payload) {
	struct ScalerResponse *scaler_response = (struct ScalerResponse*)payload;
	scaler_response->status = 1;
	scaler_response->size = 0;
	return sizeof(struct ScalerResponse);
}


uint32_t FillScalerResponse(char *payload, uint32_t size, uint32_t index) {
	struct ScalerResponse *scaler_response = (struct ScalerResponse*)payload;
	scaler_response->status = 0;
	scaler_response->size = size;
	char *scaler_data = payload + sizeof(struct ScalerResponse);
	if (index + size <= kMaxScalerArraySize) {
		memcpy(scaler_data, scaler_array+index, sizeof(struct ScalerData)*size);
	} else {
		// data divided into two parts ,first part at the end of the array
		// and second part at the beginning of the array.
		uint32_t first_part_size = kMaxScalerArraySize - index;
		uint32_t second_part_size = size - first_part_size;
		uint32_t first_part_bytes = sizeof(struct ScalerData) * first_part_size;

		memcpy(scaler_data, scaler_array+index, first_part_bytes);
		memcpy(scaler_data+first_part_bytes, scaler_array, sizeof(struct ScalerData)*second_part_size);
	}
	return (sizeof(struct ScalerResponse) + sizeof(struct ScalerData)*size);
}


volatile sig_atomic_t keep_going = 1;
int sockfd = -1;
void SigIntHandler(int) {
	if (sockfd != -1) {
		close(sockfd);
	}
	Log("Press Ctrl+C to quit.", kInfo);
	printf("You press Ctrl+C to quit.\n");
	keep_going = 0;
}


// get the scaler frequency, in counts/seconds (i.e. Hz)
void GetScalerValue(uint32_t *scaler_value) {
// for (uint32_t i = 0; i != kScalerNum; ++i) scaler_value[i] = i * 1000;	////!!!!!!!!MUST COMMENT THIS WHEN RUNNING!!!!!!//// 
// return;

	for (uint32_t i = 0; i < kScalerNum; ++i) {
		scaler_value[i] = (mapped[kScalersOffset+i] >> 12) & 0xfffff;
	}

	return;
}


// refresh screen cyclically function, call by pthread_create
void *RefreshScreen(void*) {
	signal(SIGINT, SigIntHandler);
	uint32_t scaler_value[kScalerNum];
	while (keep_going) {
		Log("RefreshScreen start of loop", kDebug);
		if (system("clear") != 0) {
			char msg[256];
			sprintf(msg, "Use bash command clear: %s", strerror(errno));
			Log(msg, kError);
			fprintf(stderr, "Error: %s\n", msg);
			exit(-1);
		}
		printf("scalar      counts\n");
		GetScalerValue(scaler_value);
		for (uint32_t i = 0; i != kScalerNum; ++i) {
			printf("%2d%15d\n", i, scaler_value[i]);
		}
		Log("RefreshScreen end of loop", kDebug);
		usleep(1000000);
	}
	return NULL;
}


// update Scaler array cyclically function, call by pthread_creates
void *UpdateScalerArray(void *vargp) {
	signal(SIGINT, SigIntHandler);
	time_t mark_time = *((time_t*)vargp);
	time_t now_time = time(NULL);
	uint64_t seconds = now_time - mark_time;
	while (seconds % kRecordPeriod != 0) {
		usleep(900000);
		now_time = time(NULL);
		seconds = now_time - mark_time;
		if (kDebug <= log_level) {
			char msg[256];
			sprintf(msg, "now second is %llu\n", (long long unsigned int)seconds);
			Log(msg, kDebug);
		}
	}
	// now the second is mutiple of RecordPeriod, and then update the Scaler array cyclically
	while (keep_going) {

		// 	calculate the scalr rates
		uint32_t scaler_value[kScalerNum];
		GetScalerValue(scaler_value);

		// lock the array and update
		pthread_mutex_lock(&scaler_array_mutex);
		uint32_t index = scaler_array_tail;
		scaler_array[scaler_array_tail].seconds = seconds;
		memcpy(scaler_array[scaler_array_tail].scaler, scaler_value, sizeof(uint32_t)*kScalerNum);
		// for (uint32_t i = 0; i != kScalerNum; i++) {
		// 	scaler_array[scaler_array_tail].Scaler[i] = ScalerRate[i];
		// }
		if (scaler_array_size == kMaxScalerArraySize) {			// array is full
			scaler_array_tail++;
			scaler_array_tail = scaler_array_tail == kMaxScalerArraySize ? 0 : scaler_array_tail;
			scaler_array_head = scaler_array_tail;
		} else {												// array not full
			scaler_array_size++;
			scaler_array_tail++;
			scaler_array_tail = scaler_array_tail == kMaxScalerArraySize ? 0 : scaler_array_tail;
		}

		if (kDebug <= log_level) {
			char msg[1024];
			sprintf(msg, "Record: %llu seconds since mark time, %s", (long long unsigned int)scaler_array[index].seconds, ctime(&now_time));
			sprintf(msg+strlen(msg), "    scaler array: size %d, head %d, tail %d.\n", scaler_array_size, scaler_array_head, scaler_array_tail);
			for (int i = 0; i != kScalerNum; ++i) {
				sprintf(msg+strlen(msg), "    %2d    %d\n", i, scaler_array[index].scaler[i]);
			}
			Log(msg, kDebug);
		}

		pthread_mutex_unlock(&scaler_array_mutex);


		usleep(kRecordPeriod*1000000-10000);			// 10ms for the program latency
		now_time = time(NULL);
		seconds = now_time - mark_time;
		while (seconds % kRecordPeriod != 0) {
			usleep(100000);							// wait for 100ms to let sec be multiple of RecordPeriod
			now_time = time(NULL);
			seconds = now_time - mark_time;
		}
	}
	return NULL;
}




void PrintUsage(const char *name) {
	printf("Usage: %s  port [log file] [log level]\n", name);
	printf("  port         Set the listening port.\n");
	printf("  log file     Set the log file path, do not log if not set.\n");
	printf("  log level    Set the log level: error, warning, info, debug, default is warning.\n");
	return;
}

// int sockfd = -1;


int main(int argc, char **argv) {
	signal(SIGINT, SigIntHandler);

	if (argc < 2 || argc > 4) {
		fprintf(stderr, "Error: Invalid argument number.\n");
		PrintUsage(argv[0]);
		return -1;
	}
	switch (argc) {
		case 2:
			if (strcmp(argv[1], "-h") == 0) {
				PrintUsage(argv[0]);
				return 0;
			}
			save_log = false;
			break;
		case 3:
			save_log = true;
			strcpy(log_file_name, argv[2]);
			break;
		case 4:
			save_log = true;
			strcpy(log_file_name, argv[2]);
			if (strcmp(argv[3], "error") == 0) {
				log_level = kError;
			} else if (strcmp(argv[3], "warning") == 0) {
				log_level = kWarning;
			} else if (strcmp(argv[3], "info") == 0) {
				log_level = kInfo;
			} else if (strcmp(argv[3], "debug") == 0) {
				log_level = kDebug;
			} else {
				fprintf(stderr, "Error: Invalid log level %s. Argument 3 should be one of the following:\nerror, warning, info, debug\n", argv[3]);
				return -1;
			}
			break;
		default:
			fprintf(stderr, "Error: Should not be here: %s line %d.\n", __FILE__, __LINE__);
			return -1;
	}


    const int memory_size = 4096;

    // open file
    int xillybus_lite_fd = open("/dev/uio0", O_RDWR);
    if (xillybus_lite_fd < 0) {
		char msg[256];
		sprintf(msg, "Failed to open /dev/uio0: %s", strerror(errno));
		Log(msg, kError);
        fprintf(stderr, "Error: %s\n", msg);
        return -1;
    }
    // lock the address space
    if (flock(xillybus_lite_fd, LOCK_EX | LOCK_NB)) {
		char msg[256];
		sprintf(msg, "Failed to get the file lock on /dev/ui0: %s", strerror(errno));
		Log(msg, kError);
        fprintf(stderr, "Error: %s\n", msg);
        return -2;
    }
    void *map_addr = mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, xillybus_lite_fd, 0);
    if (map_addr == MAP_FAILED) {
		char msg[256];
		sprintf(msg, "Failed to mmap: %s", strerror(errno));
		Log(msg, kError);
        fprintf(stderr, "Error: %s\n", msg);
        return -3;
    }
    mapped = (uint32_t*)map_addr;

	
	// set the time zero point 2022.06.01 00:00:00
	struct tm date20220601;
	date20220601.tm_year = 122;
	date20220601.tm_mon = 5;
	date20220601.tm_mday = 1;
	date20220601.tm_hour = 0;
	date20220601.tm_min = 0;
	date20220601.tm_sec = 0;
	time_t mark_time = mktime(&date20220601);
	char msg[64];
	sprintf(msg, "Set mark time %s", ctime(&mark_time));
	Log(msg, kDebug);


	// inititalize the mutex
	int mutex_init = pthread_mutex_init(&scaler_array_mutex, NULL);
	if (mutex_init != 0) {
		char msg[256];
		sprintf(msg, "Failed to initialize scaler array mutex: %s", strerror(errno));
		Log(msg, kError);
        fprintf(stderr, "Error: %s\n", msg);
		return -4;
	}

	// initialize the Scaler array
	scaler_array_head = 0;
	scaler_array_tail = 0;
	scaler_array_size = 0;

	
	struct addrinfo hints, *servinfo, *p;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;				// set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;				// set localhost IP


	int rv = getaddrinfo(NULL, argv[1], &hints, &servinfo);
	if (rv != 0) {
		char msg[256];
		sprintf(msg, "getaddrinfo: %s", gai_strerror(rv));
		Log(msg, kError);
		fprintf(stderr, "Error: %s\n", msg);
		exit(-1);
	}
	// loop to find available socket
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			char msg[256];
			sprintf(msg, "Server faild to socket: %s", strerror(errno));
			Log(msg, kWarning);
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			char msg[256];
			sprintf(msg, "Server faild to bind: %s", strerror(errno));
			Log(msg, kWarning);
			continue;
		}

		break;			// success to bind one of the sockets
	}
	freeaddrinfo(servinfo);
	if (p == NULL) {		// loop all but failed
		Log("Server failed to bind", kError);
		fprintf(stderr, "Error: Server failed to bind\n");
		exit(-1);
	}
	Log("server: waiting for recvfrom...", kInfo);


	// update the Scaler array cyclically
	pthread_t update_scaler_array_thread;
	pthread_create(&update_scaler_array_thread, NULL, UpdateScalerArray, (void*)&mark_time);


	// refresh the screen cyclically
 	// pthread_t refresh_screen_thread;
	// pthread_create(&refresh_screen_thread, NULL, RefreshScreen, NULL);


	struct sockaddr_storage their_addr;
	socklen_t addr_len = sizeof their_addr;
	char response[kBufferSize], request[kBufferSize];
	while (keep_going) {
		// receive request
		int numbytes = recvfrom(sockfd, request, kBufferSize-1, 0, (struct sockaddr*)&their_addr, &addr_len);
		if (numbytes == -1) {
			char msg[256];
			sprintf(msg, "Server recvfrom: %s", strerror(errno));
			Log(msg, kWarning);
			continue;
		}

		// display packet information
		char dst[INET6_ADDRSTRLEN];
		if (kDebug <= log_level) {
			inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), dst, sizeof dst);
			char msg[256];
			sprintf(msg, "[Debug] server: got packet from %s and it's %d bytes long\n", dst, numbytes);
			Log(msg, kDebug);
		}


		// check header
		struct RequestHeader *header = (struct RequestHeader*)request;

		if (header->type == kTypeDateRequest) {						// date request
			time_t now_time = time(NULL);
			if (kDebug <= log_level) {
				char msg[256];
				sprintf(msg, "server: now time is %s", ctime(&now_time));
				Log(msg, kDebug);
			}

			// calculate seconds since mark time point
			uint64_t sec = now_time - mark_time;

			// loan the response
			struct DateResponse *dateResponse = (struct DateResponse*)response;
			dateResponse->seconds = sec;

			// send response
			ssize_t numbytes = sendto(sockfd, response, sizeof(struct DateResponse), 0, (struct sockaddr*)&their_addr, addr_len);
			if (numbytes == -1) {
				char msg[256];
				sprintf(msg, "Server sendto date response: %s", strerror(errno));
				Log(msg, kWarning);
				continue;
			}
			if (kDebug <= log_level) {
				char msg[256];
				sprintf(msg, "Server sent packet to %s and %d bytes long\n", dst, uint32_t(sizeof(struct DateResponse)));
				Log(msg, kDebug);
			}

		} else if (header->type == kTypeScalerRequest) {												// Scaler request
			struct ScalerRequest *ScalerRequest = (struct ScalerRequest*)request;
			if (kDebug <= log_level) {
				char msg[256];
				time_t requestTime = mark_time + ScalerRequest->seconds;
				sprintf(msg, "Server get request for array of %d Scaler rates starts from %s", ScalerRequest->size, ctime(&requestTime));
				Log(msg, kDebug);
			}


			// search for requested time
			uint64_t request_seconds = ScalerRequest->seconds;
			uint32_t request_size = ScalerRequest->size;
			pthread_mutex_lock(&scaler_array_mutex);
			uint64_t head_seconds = scaler_array[scaler_array_head].seconds;
			uint64_t tail_seconds = head_seconds + (scaler_array_size-1) * kRecordPeriod;

			if (request_seconds <= tail_seconds) {																// request <= tail
				// fill Scaler response
				uint32_t index = scaler_array_head;
				index += request_seconds < head_seconds ? 0 : (request_seconds - head_seconds) / kRecordPeriod;		// request < head ?
				index %= kMaxScalerArraySize;																	// roll back
				if (request_seconds + (request_size-1) * kRecordPeriod > tail_seconds) {							// request size over the record 
					request_size = (tail_seconds - request_seconds) / kRecordPeriod + 1;
				}
				uint32_t sendbytes = FillScalerResponse(response, request_size, index);

				// edit the response status if request < head (actually not necessary in current stage)
				if (request_seconds < head_seconds) {
					struct ScalerResponse *sp = (struct ScalerResponse*)response;
					sp->status = 2;
				}

				ssize_t numbytes = sendto(sockfd, response, sendbytes, 0, (struct sockaddr*)&their_addr, addr_len);
				if (numbytes == -1) {
					char msg[256];
					sprintf(msg, "Server sendto whole scaler response: %s", strerror(errno));
					Log(msg, kWarning);
					pthread_mutex_unlock(&scaler_array_mutex);
					continue;
				}
				if (kDebug <= log_level) {
					char msg[256];
					sprintf(msg, "Server sendto Scaler response with %d bytes long and data size %d from array index %d\n", (int)numbytes, request_size, index);
					Log(msg, kDebug);
				}		
	
			} else {																							// tail < request 
				uint32_t sendbytes = FillEmptyScalerResponse(response);
				ssize_t numbytes = sendto(sockfd, response, sendbytes, 0, (struct sockaddr*)&their_addr, addr_len);
				if (numbytes == -1) {
					char msg[256];
					sprintf(msg, "Server sendto empty scaler response: %s", strerror(errno));
					Log(msg, kWarning);
					pthread_mutex_unlock(&scaler_array_mutex);
					continue;
				}

				if (kDebug <= log_level) {
					char msg[256];
					sprintf(msg, "Server sendto empty scaler response in %d bytes long.\n", (int)numbytes);
					Log(msg, kDebug);
				}
			}
			pthread_mutex_unlock(&scaler_array_mutex);
		} else {
			char msg[256];
			sprintf(msg, "Error: Unknown header type %d\n", header->type);
			Log(msg, kWarning);
			continue;
		}


	}
	close(sockfd);

	// pthread_join(refresh_screen_thread, NULL);
	pthread_join(update_scaler_array_thread, NULL);
	if (save_log && log_file) {
		fclose(log_file);
	}

	return 0;
}
