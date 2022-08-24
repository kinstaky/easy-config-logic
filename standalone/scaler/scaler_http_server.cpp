#include <ctime>
#include <cstdint>

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "external/httplib.h"
#include "external/json.hpp"

const size_t kScalerNum = 32;

std::string data_path;


size_t SearchIndex(std::ifstream &fin, uint64_t search) {
	uint64_t read_time;

	fin.seekg(0);
	fin.read((char*)&read_time, sizeof(uint64_t));

	size_t index = search - read_time;
	fin.seekg(index * (sizeof(uint64_t) + sizeof(uint32_t)*kScalerNum));
	fin.read((char*)&read_time, sizeof(uint64_t));
	if (read_time != search) {
		return -1ull;
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
	auto start = std::chrono::high_resolution_clock::now();

	response.set_header("Access-Control-Allow-Origin", "*");
	
	nlohmann::json body = nlohmann::json::parse(request.body);
	// std::cout << body.dump(2) << std::endl;

	time_t start_time = body["start"];
	time_t end_time = body["end"];
	// std::cout << "request scaler size " << body["scalers"].size() << std::endl;

	char start_file_name[128];
	sprintf(start_file_name, "%s/", data_path.c_str());
	strftime(start_file_name+strlen(start_file_name), 32, "%Y%m%d.bin", localtime(&start_time));
	char end_file_name[128];
	sprintf(end_file_name, "%s/", data_path.c_str());
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
	
	if (first_start_index == -1ull || first_end_index == -1ull) {
		// check index
		response_json["status"] = 1;
	} else {
		for (size_t i = 0; i < kScalerNum; ++i) {
			response_json["scalers"].push_back(nlohmann::json::array());
		}
		if (ReadData(first_fin, first_start_index, first_end_index, response_json)) {
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

		if (second_end_index == -1ull) {
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
	auto stop = std::chrono::high_resolution_clock::now();
	std::cout << "Handle realtime request cost " << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() << " ms" << std::endl;
}

void HandleHistoryRequest(const httplib::Request &request, httplib::Response &response) {
	auto start = std::chrono::high_resolution_clock::now();

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
		sprintf(file_name, "%s/", data_path.c_str());
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
	auto stop = std::chrono::high_resolution_clock::now();
	std::cout << "Handle history request cost " << std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() << " ms" << std::endl;
	return;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " path" << std::endl;
		std::cout << "  path    path of binary data file." << std::endl;
		return -1;
	}
	data_path = std::string(argv[1]);

	httplib::Server server;

	server.Post("/history", HandleHistoryRequest);

	server.Post("/realtime", HandleRealtimeRequest);


	server.listen("localhost", 12308);
	return 0;
}