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





void PrintVersion() {
	printf("scaler_client 2.0\n");
	printf("Part of the easy-config-logic, produced by pwl");
	return;
}


void PrintUsage(const char *name) {
	printf("Usage: %s [options] host port [data path] [log file] [log level] \n", name);
	printf("Options:\n");
	printf("  -h           Print this help and exit.\n");
	printf("  -v           Print version and exit.\n");
	printf("  -t           Record scaler values in text(in binary default).\n");
	// printf("  -p           Print scaler value on screen.\n");
	// printf("  -s           Run http server for visual scaler values.\n");
	printf("  host         Set the host to connect to, necessary.\n");
	printf("  port         Set the port to connect to, necessary.\n");
	printf("  data path    Set the path to record data and time, default is this directory(.).\n");
	printf("  log file     Set the log file path, do not log if not set.\n");
	printf("  log level    Set the log level: error, warning, info or debug, default is warning.\n");
	return;
}


// flags
bool save_scaler_in_binary = true;
bool print_on_screen = false;
bool run_http_server = false;

void ParseArguments(int argc, char **argv, int &pos_argc, char **&pos_argv) {
	pos_argc = argc-1;
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			for (size_t j = 1; j < strlen(argv[i]); ++j) {
				if (argv[i][j] == 'h') {
					PrintUsage(argv[0]);
					exit(0);
				}
				if (argv[i][j] == 'v') {
					PrintVersion();
					exit(0);
				}
				if (argv[i][j] == 't') {
					save_scaler_in_binary = false;
				} else if (argv[i][j] == 'p') {
					print_on_screen = true;
				} else if (argv[i][j] == 's') {
					run_http_server = false;
				} else {
					fprintf(stderr, "Error: Invalid option: %c", argv[i][j]);
				}
			}
		} else {
			// option must in front of positional arguments
			break;
		}
		--pos_argc;
	}
	pos_argv = argv + (argc - pos_argc);
	return;
} 



int WriteScalerValueText(uint32_t size, char *raw, uint64_t *record_time_ptr) {
	// generate file name according to the date
	time_t now_time = time(NULL);
	char file_name[128];
	sprintf(file_name, "%s/", data_path);
	strftime(file_name+strlen(file_name), 32, "%Y%m%d.txt", localtime(&now_time));

	// open file
	FILE *data_file = fopen(file_name, "a");
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
	tm *now_tm = localtime(&now_time);
	int local_hour = now_tm->tm_hour;
	int local_min = now_tm->tm_min;
	int local_sec = now_tm->tm_sec;
	size_t now_seconds = ((local_hour * 60 + local_min) * 60) + local_sec;
	size_t first_second_today = (size_t)now_time - now_seconds;
	
	struct ScalerData *data = (struct ScalerData *)raw; 
	size_t index = data[size-1].seconds - first_second_today;

	char file_name[128];
	sprintf(file_name, "%s/", data_path);
	strftime(file_name+strlen(file_name), 32, "%Y%m%d.bin", localtime(&now_time));

	if (access(file_name, F_OK)) {
		// file not exists, create it
		FILE *file = fopen(file_name, "wb");
		uint64_t fill_second = (uint64_t)now_time - now_seconds;
		uint32_t empty_scalers[32];
		memset((char*)empty_scalers, 0, sizeof(uint32_t)*32);
		// fill time and scalers at first as place holders
		for (uint64_t i = 0; i < 86400; i++) {
			fwrite((char*)&fill_second, sizeof(uint64_t), 1, file);
			fwrite((char*)empty_scalers, sizeof(uint32_t), kScalerNum, file);
			fill_second++;
		}
		fclose(file);
	}


	// open file
	FILE *data_file = fopen(file_name, "rb+");
	if (data_file == NULL) {
		char msg[256];
		sprintf(msg, "Client open binary data file %s", strerror(errno));		
		Log(msg, kError);
		fprintf(stderr, "Error: %s\n", msg);
		return -1;
	}
	
	// write data
	// size of one data point
	size_t offset = sizeof(uint64_t) + sizeof(uint32_t) * kScalerNum;
	fseek(data_file, offset * index, SEEK_SET);
	fwrite(raw, sizeof(struct ScalerData)*size, 1, data_file);

	
	if (kDebug <= log_level) {
		char msg[256];
		sprintf(msg, "Write seconds begin from %llu, size %d\n", (long long unsigned int)data[0].seconds, size);
		Log(msg, kDebug);
	}
	fclose(data_file);

	*record_time_ptr = data[size-1].seconds + kRecordPeriod;
	// printf("write time %lu at index %lu with size %u, scaler 0 is %u\n", data[size-1].seconds, index, size, data[size-1].scaler[0]);

	return 0;
}


int WriteScalerValue(uint32_t size, char *raw, uint64_t *record_time_ptr) {
	if (save_scaler_in_binary) {
		return WriteScalerValueBinary(size, raw, record_time_ptr);
	} else {
		return WriteScalerValueText(size, raw, record_time_ptr);
	}
}


volatile sig_atomic_t keep_running = 1;
int sockfd = -1;
void SigIntHandler(int) {
	if (sockfd != -1) {
		close(sockfd);
	}
	Log("Press Ctrl+C to quit.", kInfo);
	printf("You press Ctrl+C to quit.\n");
	keep_running = 0;
}


int main(int argc, char **argv) {
	signal(SIGINT, SigIntHandler);
	
	int pos_argc;
	char **pos_argv;
	ParseArguments(argc, argv, pos_argc, pos_argv);

	if (pos_argc < 2 || pos_argc > 5) {
		fprintf(stderr, "Error: Invalid argument number.\n");
		PrintUsage(argv[0]);
		return -1;
	}

	char *host_name = pos_argv[0];
	char *port_name = pos_argv[1];
	strcpy(data_path, ".");
	if (pos_argc > 2) {
		// set data path
		strcpy(data_path, pos_argv[2]);
		
		if (pos_argc > 3) {
			// save log
			save_log = true;
			strcpy(log_file_name, pos_argv[3]);

			if (pos_argc > 4) {
				// save log in specific level
				if (SetLogLevel(pos_argv[4], log_level) != 0) {
					PrintUsage(argv[0]);
					return -1;
				}
			}
		}
	}

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
	for (size_t i = 0; i != 3 && keep_running; ++i) {
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
		
		uint64_t client_time = (receive_time - send_time) / 2 + send_time;
		uint64_t server_time = date_response->seconds;
		uint64_t offset = client_time - server_time;
		offset_num++;
		offset_sum += offset;

		if (kInfo <= log_level) {
			time_t client_time_t = (time_t)((send_time+receive_time)/2);
			time_t server_time_t = (time_t)(server_time);
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
	while (keep_running) {
		// get log seconds
		uint64_t record_time = LoadRecordTime();
		if (record_time == 0) {
			time_t now_time = time(NULL);
			uint64_t now_seconds = now_time - offset_time;
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
		char msg[256];
		sprintf(msg, "Client get response status %u\n", response_status);
		Log(msg, kDebug);

		if (response_status == 0 || response_status == 2) {

			// 0 - normal response, 2 - beginning of array response.(The request
			// time was outdated, so the server gave back serveral scalar data at
			// he beginning of the array.)
			// write the scalar data to file
			int status = WriteScalerValue(scaler_response->size, response+sizeof(ScalerResponse), &record_time);
			if (!status) {
				Log("WriteScalerValue success.\n", kDebug);
				SaveRecordTime(record_time);
			}

			// check if there are more data
			time_t now_time = time(NULL);
			uint64_t now_seconds = now_time;
			// printf("record time %lu, offset_time %lu, now_seconds %lu\n", record_time, offset_time, now_seconds);
			// there are more data to access, no need to wait
			if (record_time + offset_time < now_seconds) {
				Log("Client can get more data.\n", kDebug);
				continue;
			}
		} else if (response_status == 1) {									// empty response
			// do nothing

		} else {
			char msg[256];
			sprintf(msg, "Client get invalid scalar response status %d", response_status);
			Log(msg, kWarning);
			continue;
		}

		

		// otherwise wait for some time
		uint64_t last_seconds = time(NULL);
		usleep(kRecordPeriod * kScalerStackSize * 1000000 - 30000);
		uint64_t now_seconds = time(NULL);
		while (now_seconds < last_seconds + kRecordPeriod * kScalerStackSize) {
			usleep(100000);
			now_seconds = time(NULL);
		}
	}



	freeaddrinfo(servinfo);
	close(sockfd);
	if (save_log && log_file) {
		fclose(log_file);
	}
	return 0;
}