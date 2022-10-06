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

#include <chrono>
#include <fstream>
#include <iostream>

#include "external/httplib.h"
#include "external/json.hpp"

#include "scaler_defs.h"

//-----------------------------------------------------------------------------
//	 								log
//-----------------------------------------------------------------------------

bool save_log = false;
char log_file_name[128];
enum LogLevel log_level = kWarning;
FILE *log_file = NULL;
pthread_mutex_t log_mutex;

void Log(const char *info, enum LogLevel level) {
	if (!save_log || level > log_level) {
		return;
	}
	pthread_mutex_lock(&log_mutex);
	if (!log_file) {
		log_file = fopen(log_file_name, "w");
		if (!log_file) {
			fprintf(stderr, "Error: Could not open log file %s.\n", log_file_name);
			pthread_mutex_unlock(&log_mutex);
			exit(-1);
		}
	}
	time_t now_time = time(NULL);
	fprintf(log_file, "%s[%c%s] %s\n", ctime(&now_time), log_level_name[level][0]-32, log_level_name[level]+1, info);
	fflush(log_file);
	pthread_mutex_unlock(&log_mutex);
	return;
}


//-----------------------------------------------------------------------------
//	 								record time
//-----------------------------------------------------------------------------

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


//-----------------------------------------------------------------------------
//	 								arguments
//-----------------------------------------------------------------------------

void PrintVersion() {
	printf("scaler_client 2.1.1\n");
	printf("Part of the easy-config-logic, produced by pwl\n");
	return;
}


void PrintUsage(const char *name) {
	printf("Usage: %s [options] host port [data path] [log file] [log level] \n", name);
	printf("Options:\n");
	printf("  -h           Print this help and exit.\n");
	printf("  -v           Print version and exit.\n");
	printf("  -t           Record scaler values in text(in binary default).\n");
	printf("  -p           Print scaler value on screen.\n");
	printf("  -s           Run http server for visual scaler values.\n");
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
					run_http_server = true;
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


//-----------------------------------------------------------------------------
//	 								write value to file
//-----------------------------------------------------------------------------

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


//-----------------------------------------------------------------------------
//	 								signal
//-----------------------------------------------------------------------------

volatile sig_atomic_t keep_running = 1;
int sockfd = -1;
httplib::Server server;
void SigIntHandler(int) {
	// close socket
	if (sockfd != -1) {
		close(sockfd);
	}
	Log("Press Ctrl+C to quit.", kInfo);
	printf("You press Ctrl+C to quit.\n");
	
	// stop main loop
	keep_running = 0;
	
	// switch to primary screen
	if (print_on_screen) {
		if (system("tput rmcup") != 0) {
			Log("tput rmcup failed.", kError);
			printf(
				"Please type `tput rmcup` or printf '\e[2J\e[?47l\e8'` by"
				"yourself to switch back to the primary screen if you are in the"
				"second screen.\n"
			);
		}
	}

	if (run_http_server) {
		server.stop();
	}
}


//-----------------------------------------------------------------------------
//	 							display on screen
//-----------------------------------------------------------------------------
pthread_mutex_t scaler_array_mutex;
uint32_t current_scaler_value[kScalerNum];

void* RefreshScreen(void*) {
	if (system("tput smcup") != 0) {
		Log("bash command tput smcup failed.", kError);
		fprintf(stderr, "Error: bash command tput smcup failed.\n");
	}
	while (keep_running) {
		if (system("clear") != 0) {
			Log("system clear failed.", kError);
			return NULL;
		}
		printf("scalar      counts\n");
		pthread_mutex_lock(&scaler_array_mutex);
		for (size_t i = 0; i < kScalerNum; i++) {
			printf("%2u%15d\n", (uint32_t)i, current_scaler_value[i]);
		}
		pthread_mutex_unlock(&scaler_array_mutex);
		usleep(500000);
	}
	return NULL;
}

void UpdateScalerValue(uint32_t size, struct ScalerData *data) {
	pthread_mutex_lock(&scaler_array_mutex);
	for (size_t i = 0; i < kScalerNum; ++i) {
		current_scaler_value[i] = data[size-1].scaler[i];
	}
	pthread_mutex_unlock(&scaler_array_mutex);
}


//-----------------------------------------------------------------------------
//	 								http server
//-----------------------------------------------------------------------------

const size_t kInvalidIndex = 86401;

size_t SearchIndex(std::ifstream &fin, uint64_t search) {
	uint64_t read_time;

	fin.seekg(0);
	fin.read((char*)&read_time, sizeof(uint64_t));

	size_t index = search - read_time;
	fin.seekg(index * (sizeof(uint64_t) + sizeof(uint32_t)*kScalerNum));
	fin.read((char*)&read_time, sizeof(uint64_t));
	if (read_time != search) {
		std::cerr << "Warning: read time " << read_time << ", search " << search << "\n";
		return kInvalidIndex;
	}
	return index;
}


int ReadData(std::ifstream &fin, size_t start, size_t end, nlohmann::json &json) {
	const size_t offset = sizeof(uint64_t) + sizeof(uint32_t) * kScalerNum;
		
	uint32_t scalers[kScalerNum];
	for (size_t i = start; i < end; ++i) {
		// read data
		fin.seekg(offset*i + sizeof(uint64_t));
		fin.read((char*)scalers, sizeof(uint32_t)*kScalerNum);
		// fill data
		for (size_t j = 0; j < kScalerNum; ++j) {
			json["scalers"][j].push_back(scalers[j]);
		}
	}

	return 0;
}


void HandleRealtimeRequest(const httplib::Request &request, httplib::Response &response) {
	// auto start = std::chrono::high_resolution_clock::now();

	response.set_header("Access-Control-Allow-Origin", "*");
	
	nlohmann::json body = nlohmann::json::parse(request.body);
	// std::cout << body.dump(2) << std::endl;

	time_t start_time = body["start"];
	time_t end_time = body["end"];
	// std::cout << "request scaler size " << body["scalers"].size() << std::endl;

	char start_file_name[128];
	sprintf(start_file_name, "%s/", data_path);
	strftime(start_file_name+strlen(start_file_name), 32, "%Y%m%d.bin", localtime(&start_time));
	char end_file_name[128];
	sprintf(end_file_name, "%s/", data_path);
	strftime(end_file_name+strlen(end_file_name), 32, "%Y%m%d.bin", localtime(&end_time));

	bool same_file = strcmp(start_file_name, end_file_name) == 0;
	
	nlohmann::json response_json;
	response_json["status"] = 0;


	std::ifstream first_fin(start_file_name, std::ios::binary);
	if (!first_fin.good()) {
		std::cerr << "Warning: open file " << start_file_name << " failed." << std::endl;
		std::cerr << "Ignore this request." << std::endl;
		return;
	}
	
	// get the start index in first file
	size_t first_start_index = SearchIndex(first_fin, start_time);
	// end index in first file is the last one if data is in two files
	// or search for it if all data in one file
	size_t first_end_index = same_file ? SearchIndex(first_fin, end_time) : 86400;
	// std::cout << "first start index " << first_start_index
	// 	<< " first end index " << first_end_index << std::endl;
	
	if (first_start_index == kInvalidIndex || first_end_index == kInvalidIndex) {
		// check index
		std::cerr << "Warning: Invalid index\n";
		response_json["status"] = 1;
	} else {
		for (size_t i = 0; i < kScalerNum; ++i) {
			response_json["scalers"].push_back(nlohmann::json::array());
		}
		if (ReadData(first_fin, first_start_index, first_end_index, response_json)) {
			std::cerr << "Warning: read data failed.\n";
			response_json["status"] = 1;
		}
	}
	first_fin.close();

	// second file
	if (!same_file) {
		std::ifstream second_fin(end_file_name, std::ios::binary);
		if (!second_fin.good()) {
			std::cerr << "Warning: open file " << end_file_name << " failed." << std::endl;
			std::cerr << "Ignore this request." << std::endl;
			return;
		}
		
		// the second file always start from 0
		size_t second_start_index = 0;
		// suppose file number won't be over two
		size_t second_end_index = SearchIndex(second_fin, end_time);

		if (second_end_index == kInvalidIndex) {
			// check index
			response_json["status"] = 1;			
		} else {
			if (ReadData(second_fin, second_start_index, second_end_index, response_json) ) {
				response_json["status"] = 1;
			}
		}
		second_fin.close();
	}

	// std::cout << response_json.dump(2) << std::endl;
	response.set_content(response_json.dump(), "text/plain");

	// time cost
	// auto stop = std::chrono::high_resolution_clock::now();
	// std::cout << "Handle realtime request cost " << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() << " ms" << std::endl;
}

void HandleHistoryRequest(const httplib::Request &request, httplib::Response &response) {
	// auto start = std::chrono::high_resolution_clock::now();

	// CORS
	response.set_header("Access-Control-Allow-Origin", "*");
	
	nlohmann::json body = nlohmann::json::parse(request.body);
	std::cout << body.dump(2) << std::endl;

	// calculate start and end date
	std::string start_date = body["start"];
	std::string end_date = body["end"];
	std::cout << start_date << " " << end_date << std::endl;
	tm start_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	strptime(start_date.c_str(), "%Y-%m-%d", &start_time);
	time_t start_seconds = mktime(&start_time);
	struct tm end_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	strptime(end_date.c_str(), "%Y-%m-%d", &end_time);
	time_t end_seconds = mktime(&end_time);
	size_t request_days = difftime(end_seconds, start_seconds) / 86400 + 1;
	
	// get today's date
	time_t now_seconds = time(NULL);
	tm *now_time = localtime(&now_seconds);
	char now_date[16];
	strftime(now_date, 16, "%Y-%m-%d", now_time);
	bool include_today = strcmp(now_date, end_date.c_str()) == 0;

	nlohmann::json response_json;
	response_json["status"] = 0;
	for (size_t i = 0; i < kScalerNum; ++i) {
		response_json["scalers"].push_back(nlohmann::json::array());
	}

	// read data except for today
	for (size_t day = 0; day < request_days; ++day) {
		// get file name
		time_t loop_seconds = start_seconds + 86400*day;
		char file_name[128];
		sprintf(file_name, "%s/", data_path);
		strftime(file_name+strlen(file_name), 32, "%Y%m%d.bin", localtime(&loop_seconds));

		// read data
		std::ifstream fin(file_name, std::ios::binary);
		if (!fin.good()) {
			std::cerr << "Warning: Could not open file " << file_name << std::endl;
			if (include_today && day == request_days-1) {
				for (size_t i = 0; i < kScalerNum; ++i) {
					for (auto j = loop_seconds; j < now_seconds; ++j) {
						response_json["scalers"][i].push_back(0);
					}					
				}				
			} else {
				for (size_t i = 0; i < kScalerNum; ++i) {
					for (size_t j = 0; j < 86400; ++j) {
						response_json["scalers"][i].push_back(0);
					}
				}
			}
		}
		if (include_today && day == request_days-1) {
			// can only get part of today's data
			if (ReadData(fin, 0, now_seconds-end_seconds, response_json)) {
				response_json["status"] = 1;
			}
		} else {
			if (ReadData(fin, 0, 86400, response_json)) {
				response_json["status"] = 1;
			}
		}
		fin.close();
	}

	
	response.set_content(response_json.dump(), "text/plain");

	// time cost
	// auto stop = std::chrono::high_resolution_clock::now();
	// std::cout << "Handle history request cost "
	// 	<< std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count()
	// 	<< " ms" << std::endl;
	return;
}


/// @brief run http server for the scaler visual
///
/// @returns NULL
///
void* HttpServer(void*) {
	server.Post("/history", HandleHistoryRequest);
	server.Post("/realtime", HandleRealtimeRequest);
	server.listen("localhost", 12308);
	return NULL;
}


//-----------------------------------------------------------------------------
//	 							main loop
//-----------------------------------------------------------------------------

int main(int argc, char **argv) {
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

	if (pthread_mutex_init(&log_mutex, NULL) != 0) {
		fprintf(stderr, "Error: Failed to initialize log mutex.\n");
		return -1;
	}

	pthread_t refresh_screen_thread;
	if (print_on_screen) {
		if (pthread_mutex_init(&scaler_array_mutex, NULL) != 0) {
			Log("Initialize scaler array mutex failed.\n", kError);
			fprintf(stderr, "Error: Initialize scaler array mutex failed.\n");
			print_on_screen = false;
		} else {
			// initialize mutex success, create refresh screen thread
			pthread_create(&refresh_screen_thread, NULL, RefreshScreen, NULL);
		}
	}
	
	pthread_t http_server_thread;
	if (run_http_server) {
		Log("Start http server", kInfo);
		pthread_create(&http_server_thread, NULL, HttpServer, NULL);
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


	signal(SIGINT, SigIntHandler);

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
			} else {
				// get the present scaler data, update it
				UpdateScalerValue(
					scaler_response->size,
					(struct ScalerData*)(response+sizeof(ScalerResponse))
				);
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

	if (print_on_screen) {
		pthread_join(refresh_screen_thread, NULL);
	}
	if (run_http_server) {
		pthread_join(http_server_thread, NULL);
	}

	if (save_log && log_file) {
		fclose(log_file);
	}
	return 0;
}