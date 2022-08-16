#include <errno.h>
#include <netdb.h>
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


bool save_log = false;
char log_file_name[128];
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

int SetLogLevel(char *level) {
	bool set_log_level = false;
	for (size_t i = 0; i < 4; ++i) {
		if (strcmp(level, log_level_name[i]) == 0) {
			log_level = LogLevel(kError+i);
			set_log_level = true;
			break;
		}
	}
	if (!set_log_level) {
		// this argument doesn't match any log level name
		fprintf(stderr, "Error: Invalid log level %s.", level);
		fprintf(stderr, "Log level should be one of following:\n"
			"error, warning, info, debug.\n");
		return -1;
	}
	return 0;
}


char data_path[96];
const char record_time_file_name[] = "client-recorded-time.txt";

void SaveRecordTime(uint64_t second) {
	// open time record file and write the seconds value
	char file_name[128];
	sprintf(file_name, "%s/%s", data_path, record_time_file_name);
	FILE *recorded_time_file = fopen(file_name, "w");
	if (recorded_time_file == NULL) {
		char msg[256];
		sprintf(msg, "Client open time record file failed: %s", strerror(errno));
		Log(msg, kError);
		fprintf(stderr, "Error: %s\n", msg);
		exit(-1);
	}
	fprintf(recorded_time_file, "%llu\n", (long long unsigned int)second);
	fclose(recorded_time_file);
}

uint64_t LoadRecordTime() {
	uint64_t sec;
	// opne time record file and read seconds value
	char file_name[128];
	sprintf(file_name, "%s/%s", data_path, record_time_file_name);
	FILE *recorded_time_file = fopen(file_name, "r");
	if (recorded_time_file == NULL) {
		char msg[256];
		sprintf(msg, "Client open time record file failed: %s", strerror(errno));
		Log(msg, kWarning);
		return 0;
	}
	if (fscanf(recorded_time_file, "%llu", (long long unsigned int*)&sec) != 1) {
		char msg[256];
		sprintf(msg, "Client load record time failed: %s", strerror(errno));
		Log(msg, kWarning);
		return 0;
	}
	fclose(recorded_time_file);

	return sec;
}



bool scaler_save_in_binary = true;

int WriteScalerValueText(uint32_t size, char *raw, uint64_t *record_time_ptr) {
	// generate file name according to the date
	time_t now_time = time(NULL);
	char date_file_name[128];
	sprintf(date_file_name, "%s/", data_path);
	strftime(date_file_name+strlen(date_file_name), 32, "%Y%m%d.txt", localtime(&now_time));
	
	// open file
	FILE *data_file = fopen(date_file_name, "a");
	if (data_file == NULL) {
		char msg[256];
		sprintf(msg, "Client open data file %s", strerror(errno));		
		Log(msg, kError);
		fprintf(stderr, "Error: %s\n", msg);
		return -1;
	}
	
	// write data
	struct ScalerData *data = (struct ScalerData *)raw;
	for (uint32_t i = 0; i != size; ++i) {
		fprintf(data_file, "%llu", (long long unsigned int)data[i].seconds);
		for (size_t j = 0; j != kScalerNum; ++j) {
			fprintf(data_file, "%12d", data[i].scaler[j]);
		}
		fprintf(data_file, "\n");
	}
	
	if (kDebug <= log_level) {
		char msg[256];
		sprintf(msg, "Write seconds begin from %llu, size %d\n", (long long unsigned int)data[0].seconds, size);
		Log(msg, kDebug);
	}
	fclose(data_file);

	*record_time_ptr = data[size-1].seconds + kRecordPeriod;

	return 0;
}


int WriteScalerValueBinary(uint32_t size, char *raw, uint64_t *record_time_ptr) {
	// generate file name according to the date
	time_t now_time = time(NULL);
	char date_file_name[128];
	sprintf(date_file_name, "%s/", data_path);
	strftime(date_file_name+strlen(date_file_name), 32, "%Y%m%d.bin", localtime(&now_time));
	
	// open file
	FILE *data_file = fopen(date_file_name, "ab");
	if (data_file == NULL) {
		char msg[256];
		sprintf(msg, "Client open binary data file %s", strerror(errno));		
		Log(msg, kError);
		fprintf(stderr, "Error: %s\n", msg);
		return -1;
	}
	
	// write data
	fwrite(raw, sizeof(struct ScalerData)*size, 1, data_file);

	struct ScalerData *data = (struct ScalerData *)raw; 
	if (kDebug <= log_level) {
		char msg[256];
		sprintf(msg, "Write seconds begin from %llu, size %d\n", (long long unsigned int)data[0].seconds, size);
		Log(msg, kDebug);
	}
	fclose(data_file);

	*record_time_ptr = data[size-1].seconds + kRecordPeriod;

	return 0;
}


int WriteScalerValue(uint32_t size, char *raw, uint64_t *record_time_ptr) {
	if (scaler_save_in_binary) {
		return WriteScalerValueBinary(size, raw, record_time_ptr);
	} else {
		return WriteScalerValueText(size, raw, record_time_ptr);
	}
}


volatile sig_atomic_t keep_going = 1;
int sockfd = -1;
void SigIntHandler(int) {
	if (sockfd != -1) {
		close(sockfd);
	}
	Log("Press Ctrl+C to quit.", kInfo);
	printf("You press Ctrl+C to quit.\n");
	exit(1);
}


void PrintUsage(const char *name) {
	printf("Usage: %s [options] host port [data path] [log file] [log level] \n", name);
	printf("Options:\n");
	printf("  -t           Record scaler values in text(in binary default).\n");
	printf("  -h           Print this help information.\n");
	printf("  host         Set the host to connect to, necessary.\n");
	printf("  port         Set the port to connect to, necessary.\n");
	printf("  data path    Set the path to record data and time, default is this directory(.).\n");
	printf("  log file     Set the log file path, do not log if not set.\n");
	printf("  log level    Set the log level: error, warning, info or debug, default is warning.\n");
	return;
}


int main(int argc, char **argv) {
	if (argc < 2 || argc > 7) {
		fprintf(stderr, "Error: Invalid argument number.\n");
		PrintUsage(argv[0]);
		return -1;
	}
	if (argv[1][0] == '-') {
		if (strlen(argv[1]) != 2) {
			fprintf(stderr, "Error: Unknown option %s", argv[1]);
			PrintUsage(argv[0]);
			return -1;
		}

		if (argv[1][1] == 'h') {
			PrintUsage(argv[0]);
			return 0;
		}
		if (argv[1][1] == 't') {
			scaler_save_in_binary = false;
		} else {
			fprintf(stderr, "Error: Unknown option %s", argv[1]);
			PrintUsage(argv[0]);
			return -1;
		}
	}
	int args_start = scaler_save_in_binary ? 1 : 2;
	if (argc < args_start + 2) {
		fprintf(stderr, "Error: Invalid argument number, host and port is necessary.\n");
		PrintUsage(argv[0]);
		return -1;
	}
	char *host_name = argv[args_start];
	char *port_name = argv[args_start+1];
	if (argc >= args_start + 3) {
		// set data path
		strcpy(data_path, argv[args_start+2]);
		
		if (argc >= args_start + 4) {
			// save log
			save_log = true;
			strcpy(log_file_name, argv[args_start+3]);

			if (argc >= args_start + 5) {
				// save log in specific level
				if (SetLogLevel(argv[args_start+4]) != 0) {
					PrintUsage(argv[0]);
					return -1;
				}

				if (argc > args_start + 6) {
					fprintf(stderr, "Error: Unknown argument: %s\n", argv[args_start+5]);
					PrintUsage(argv[0]);
					return -1;
				}
			}
		}
	}


	signal(SIGINT, SigIntHandler);

	// check address information
	struct addrinfo hints;
	struct addrinfo *servinfo = NULL;
	struct addrinfo *p = NULL;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	int rv = getaddrinfo(host_name, port_name, &hints, &servinfo);
	if (rv != 0) {
		char msg[256];
		sprintf(msg, "Client get addrinfo: %s", gai_strerror(rv));
		Log(msg, kError);
		fprintf(stderr, "Error: %s\n", msg);
		exit(-1);
	}

	// loop to search for appropiate ip and port
	for (p = servinfo; p != NULL; p = p->ai_next) {
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sockfd == -1) {
			char msg[256];
			sprintf(msg, "Client failed to socket: %s", strerror(errno));
			Log(msg, kWarning);
			continue;
		}
		break;
	}

	if (p == NULL) {			// none can be the socket
		char msg[256];
		sprintf(msg, "Client failed to create socket");
		fprintf(stderr, "Error: %s\n", msg);
		exit(-1);
	}

	char dst[INET6_ADDRSTRLEN];
	if (kDebug <= log_level) {
		inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), dst, sizeof dst);
		char msg[256];
		sprintf(msg, "Client prepare to send to %s", dst);
		Log(msg, kDebug);
	}

	Log("Client prepare to request...", kDebug);
		

	// set the time zero point 2022.06.01 00:00:00
	struct tm date20220601;
	date20220601.tm_year = 122;
	date20220601.tm_mon = 5;
	date20220601.tm_mday = 1;
	date20220601.tm_hour = 0;
	date20220601.tm_min = 0;
	date20220601.tm_sec = 0;
	time_t mark_time = mktime(&date20220601);
	if (kDebug <= log_level) {
		char msg[256];
		sprintf(msg, "Set mark time %s", ctime(&mark_time));
		Log(msg, kDebug);
	}

	
	// at first, get the server time and calculate the offset
	ssize_t numbytes;										// send/receive bytes
	struct sockaddr_storage their_addr;						// receive address information
	socklen_t addr_len = sizeof their_addr;					// address length
	char request[kBufferSize], response[kBufferSize];		// request and response
	size_t offset_num = 0;									// offset numbers, to calculate the average offset
	uint64_t offset_sum = 0;									// offset sum, to calculate the average offset
	// send date request for 3 times
	struct DateRequest *date_request = (struct DateRequest*)request;
	date_request->header.type = kTypeDateRequest;
	for (size_t i = 0; i != 3 && keep_going; ++i) {
		time_t send_time = time(NULL);

		// send date request
		numbytes = sendto(sockfd, request, sizeof(struct DateRequest), 0, p->ai_addr, p->ai_addrlen);
		if (numbytes == -1) {
			char msg[256];
			sprintf(msg, "Client sendto date request failed: %s", strerror(errno));
			Log(msg, kWarning);
			continue;
		}
		Log("Client sent date request.", kDebug);

		// receive date response
		numbytes = recvfrom(sockfd, response, kBufferSize-1, 0, (struct sockaddr*)&their_addr, &addr_len);
		if (numbytes == -1) {
			char msg[256];
			sprintf(msg, "Client recvform date response: %s", strerror(errno));
			Log(msg, kWarning);
			continue;
		}
		struct DateResponse *date_response = (struct DateResponse*)response;
		Log("Client receive date response.", kDebug);


		time_t receive_time = time(NULL); 
		
		uint64_t client_time = (receive_time - send_time) / 2 + (send_time - mark_time);
		uint64_t server_time = date_response->seconds;
		uint64_t offset = client_time - server_time;
		offset_num++;
		offset_sum += offset;

		if (kInfo <= log_level) {
			time_t client_time_t = (time_t)((send_time+receive_time)/2);
			time_t server_time_t = (time_t)(mark_time+server_time);
			char msg[256];
			sprintf(msg, "Client time %s", ctime(&client_time_t));
			sprintf(msg+strlen(msg), "Server time %s", ctime(&server_time_t));			
			Log(msg, kInfo);
		}

		if (i != 2) usleep(800000);					// sleep for 800ms
	}
	uint64_t offset_time = offset_sum / offset_num;
	if (kDebug <= log_level) {
		char msg[256];
		sprintf(msg, "Get client and server time offset %llu seconds.", (long long unsigned int)offset_time);
		Log(msg, kDebug);
	}

	// send scalar request periodically
	while (keep_going) {
		// get log seconds
		uint64_t record_time = LoadRecordTime();
		if (record_time == 0) {
			time_t now_time = time(NULL);
			uint64_t now_seconds = now_time - mark_time - offset_time;
			record_time = now_seconds - now_seconds % kRecordPeriod;
		}
		// fill scalar request
		struct ScalerRequest *scaler_request = (struct ScalerRequest*)request;
		scaler_request->header.type = kTypeScalerRequest;
		scaler_request->size = kScalerStackSize;
		scaler_request->seconds = record_time;

		// send scalar request
		numbytes = sendto(sockfd, request, sizeof(struct ScalerRequest), 0, p->ai_addr, p->ai_addrlen);
		if (numbytes == -1) {
			char msg[256];
			sprintf(msg, "Client sendto scaler request failed: %s", strerror(errno));
			Log(msg, kWarning);
			continue;
		}
		if (kDebug <= log_level) {
			char msg[256];
			sprintf(msg, "Client send scalar request in %u bytes, type %u, size %u, seconds %llu\n",
				(uint32_t)sizeof(struct ScalerRequest), scaler_request->header.type, scaler_request->size, (long long unsigned int)scaler_request->seconds
			);
		}

		// receive scalar response
		numbytes = recvfrom(sockfd, response, kBufferSize-1, 0, (struct sockaddr*)&their_addr, &addr_len);
		// check numbytes and response->size
		if (numbytes == -1) {
			char msg[256];
			sprintf(msg, "Client recvfrom scaler requeset failed: %s", strerror(errno));
			Log(msg, kWarning);
			continue;
		}
		struct ScalerResponse* scaler_response = (struct ScalerResponse*)response;
		if ((size_t)numbytes != sizeof(ScalerResponse) + sizeof(ScalerData)*scaler_response->size) {
			char msg[256];
			sprintf(msg, "Client contradictory bytes, numbytes %d and response->size %d\n", (int)numbytes, scaler_response->size);
			Log(msg, kWarning);
			continue;
		}

		uint32_t response_status = scaler_response->status;
		if (response_status == 0 || response_status == 2) {
			// 0 - normal response, 2 - beginning of array response.(The request time was outdated, so the server gave back serveral scalar data at he beginning of the array.)
			// write the scalar data to file
			int status = WriteScalerValue(scaler_response->size, response+sizeof(ScalerResponse), &record_time);
			if (!status) {
				SaveRecordTime(record_time);
			}

			// check if there are more data
			time_t now_time = time(NULL);
			uint64_t now_seconds = now_time - mark_time;
			if (record_time + offset_time + 10 < now_seconds) continue;				// there more date to access, no need to wait

		} else if (response_status == 1) {									// empty response
			// do nothing

		} else {
			char msg[256];
			sprintf(msg, "Client get invalid scalar response status %d", response_status);
			Log(msg, kWarning);
			continue;
		}

		

		// otherwise wait for some time
		sleep(kRecordPeriod * kScalerStackSize);
	}



	freeaddrinfo(servinfo);
	close(sockfd);
	if (save_log && log_file) {
		fclose(log_file);
	}
	return 0;
}