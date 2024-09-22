#include "service.h"

#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <ctime>

#include <sys/file.h>
#include <sys/mman.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <random>

#include <grpcpp/grpcpp.h>

#include "config/config_parser.h"
#include "config/memory_config.h"
#include "i2c.h"

namespace ecl {

bool ScalerService::keep_running = true;

ScalerService::ScalerService(const ServiceOption &option) noexcept
: port_(option.port)
, log_level_(option.log_level)
, test_(option.test)
, data_path_(option.data_path)
, device_name_(option.device_name)
, xillybus_lite_fd_(-1)
, memory_(nullptr) {

	keep_running = true;

	if (log_level_ >= kDebug) {
		std::cout << "[Debug] Initialize scaler service:\n"
			<< "  port: " << port_ << "\n"
			<< "  data path: " << data_path_ << "\n"
			<< "  device name: " << device_name_ << "\n"
			<< "  log level: " << kLogLevelName[log_level_] << "\n"
			<< "  test: " << test_ << "\n";
	}

	if (test_) {
		memory_ = new Memory;
		for (size_t i = 0; i < kMaxScalers; ++i) {
			memory_->scaler[i].value = i*100*test_;
		}
	} else {
		xillybus_lite_fd_ = open("/dev/uio0", O_RDWR);
		if (xillybus_lite_fd_ < 0) {
			if (log_level_ >= kError) {
				std::cout << "[Error] Failed to open /dev/uio0: "
					<< strerror(errno) << "\n";
			}
			exit(-1);
		}
		// lock the address space
		if (flock(xillybus_lite_fd_, LOCK_EX | LOCK_NB)) {
			if (log_level_ >= kError) {
				std::cout << "[Error] Failed to get file lock on /dev/ui0: "
					<< strerror(errno) << "\n";
			}
			exit(-1);
		}
		// get mapped adderess
		void *map_addr = mmap(
			NULL, sizeof(Memory),
			PROT_READ | PROT_WRITE, MAP_SHARED,
			xillybus_lite_fd_, 0
		);
		if (map_addr == MAP_FAILED) {
			if (log_level_ >= kError) {
				std::cout << "[Error] Failed to mmap: "
					<< strerror(errno) << "\n";
			}
			exit(-1);
		}
		// convert pointer
		memory_ = (Memory*)map_addr;
	}
	// check data path
	if (data_path_[data_path_.length()-1] != '/') {
		data_path_ += "/";
	}

	if (test_) {
		test_thread_ = std::make_unique<std::thread>(
			[&]() {
				auto next = std::chrono::steady_clock::now();
				while (keep_running) {
					std::random_device r;
					std::mt19937 engine(r());
					for (size_t i = 0; i < kMaxScalers; ++i) {
						std::normal_distribution<double> distribution(
							0.0, i*10*test_
						);
						memory_->scaler[i].value =
							i*100*test_ + std::round(distribution(engine));
					}
					next += std::chrono::seconds(1);
					std::this_thread::sleep_until(next);
				}
			}
		);
	}

	write_thread_ = std::make_unique<std::thread>(
		[&]() {
			auto next = std::chrono::steady_clock::now();
			while (keep_running) {
				WriteScaler();
				next += std::chrono::seconds(1);
				std::this_thread::sleep_until(next);
			}
		}
	);
}


ScalerService::~ScalerService() {
	if (test_) {
		if (memory_) delete memory_;
	} else {
		if (xillybus_lite_fd_ >= 0) {
			// clean up
			flock(xillybus_lite_fd_, LOCK_UN);
			munmap((void*)memory_, sizeof(Memory));
			close(xillybus_lite_fd_);
		}
	}
	if (test_) test_thread_->join();
	write_thread_->join();
	if (log_level_ >= kDebug) {
		std::cout << "[Debug] Clear scaler service successfully.\n";
	}
}


void SigIntHandler(int) {
	ScalerService::keep_running = false;
}


void ScalerService::PrintScaler() const noexcept {
	signal(SIGINT, SigIntHandler);
	if (system("tput smcup")) {
		std::cout << "[Error] Use bash command tput smcup failed.\n";
		exit(-1);
	}
	while (keep_running) {
		if (system("clear")) {
			std::cout << "[Error] Use bash command clear failed.\n";
			exit(-1);
		}
		std::cout << "scaler      counts\n";
		for (size_t i = 0; i < kMaxScalers; ++i) {
			printf("%2ld%15d\n", i, memory_->scaler[i].value);
		}
		usleep(1000000);
	}
	if (system("tput rmcup")) {
		std::cout << "[Error] tput rmcup failed.\n";
		std::cout << "[Info] Please type `tput rmcup` or printf '\e[2J\e[?47l\e8'`"
			<< " by yourself to switch back to the primary screen if you are in"
			<< " the secondary screen.\n";
	}
}


/// @brief construct file name
/// @param[in] data_path data stored path
/// @param[in] device_name device name
/// @param[in] t c style time
/// @returns file name
std::string GetFileName(
	std::string data_path,
	std::string device_name,
	tm *t
) {
	std::stringstream file_name;
	file_name << data_path << t->tm_year+1900
		<< std::setw(2) << std::setfill('0') << t->tm_mon+1
		<< std::setw(2) << std::setfill('0') << t->tm_mday
		<< (device_name.empty() ? "" : "-"+device_name)
		<< ".bin";
	return file_name.str();
}


int ScalerService::ReadDateScaler(
	tm* date,
	int32_t flag,
	size_t seconds,
	size_t size,
	int average,
	std::vector<std::vector<uint32_t>> &scalers
) const noexcept {
	// check parameters
	if (flag == 0) return -1;
	if (seconds + size*average > 86400) return -1;
	// initialize
	scalers.clear();
	std::vector<int> indexes;
	for (int32_t i = 0; i < 32; ++i) {
		if (flag & (1 << i)) {
			scalers.push_back(std::vector<uint32_t>());
			indexes.push_back(i);
		}
	}
	double sum[32];
	for (size_t i = 0; i < 32; ++i) sum[i] = 0.0;
	int sum_number = 0;

	// get file name
	std::string file_name = GetFileName(data_path_, device_name_, date);
	// open file
	std::ifstream fin(file_name, std::ios::binary);
	if (!fin.good()) {
		std::cout << "[Error] Could not open file " << file_name << "\n";
		return -2;
	}
	// get offset
	size_t offset = sizeof(ScalerFileHeader)
		+ seconds * kMaxScalers * sizeof(uint32_t);
	// set start position for reading
	fin.seekg(offset);
	// read
	uint32_t read_value[kMaxScalers];
	for (size_t i = 0; i < size*average; ++i) {
		fin.read((char*)read_value, sizeof(uint32_t)*kMaxScalers);
		++sum_number;
		for (size_t j = 0; j < indexes.size(); ++j) {
			sum[j] += read_value[indexes[j]];
			if (sum_number == average) {
				scalers[j].push_back(std::round(sum[j] / average));
				sum[j] = 0.0;
			}
		}
		if (sum_number == average) {
			sum_number = 0;
		}
	}
	// close file
	fin.close();

	return 0;
}


int ScalerService::ReadRecentScaler(
	int32_t flag,
	int seconds,
	int average,
	std::vector<std::vector<uint32_t>> &scalers
) const noexcept {
	// initialize
	scalers.clear();
	if (seconds <= 0) return 0;
	if (seconds % average) return -1;
	double sum[32];
	for (size_t i = 0; i < 32; ++i) sum[i] = 0.0;
	int sum_number = 0;
	std::vector<int> indexes;
	for (int32_t i = 0; i < 32; ++i) {
		if (flag & (1 << i)) {
			scalers.push_back(std::vector<uint32_t>());
			indexes.push_back(i);
		}
	}

	// get current time
	time_t now = time(NULL);
	tm *now_tm = localtime(&now);
	int now_second = (now_tm->tm_hour*60 + now_tm->tm_min)*60 + now_tm->tm_sec;

	if (now_second+1 < seconds) {
		// read yesterday's file data
		// construct file name
		time_t yesterday = time(NULL);
		tm *yesterday_tm = localtime(&yesterday);
		yesterday_tm->tm_mday--;
		mktime(yesterday_tm);
		std::string file_name = GetFileName(data_path_, device_name_, yesterday_tm);
		// open file
		std::ifstream fin(file_name, std::ios::binary);
		if (!fin.good()) {
			std::cout << "[Error] Could not open file " << file_name << "\n";
			return -2;
		}
		// read data
		int start = 86400 - seconds + now_second + 1;
		// get offset
		size_t offset = sizeof(ScalerFileHeader)
			+ start * kMaxScalers * sizeof(uint32_t);
		// goto start position
		fin.seekg(offset);
		// read
		uint32_t read_value[kMaxScalers];
		for (int i = 0; i < seconds-now_second-1; ++i) {
			fin.read((char*)read_value, sizeof(uint32_t)*kMaxScalers);
			++sum_number;
			for (size_t j = 0; j < indexes.size(); ++j) {
				sum[j] += read_value[indexes[j]];
				if (sum_number == average) {
					scalers[j].push_back(std::round(sum[j] / average));
					sum[j] = 0.0;
				}
			}
			if (sum_number == average) {
				sum_number = 0;
			}
		}
		// change seconds
		seconds = now_second+1;
		// close file
		fin.close();
	}

	// read today's data
	// get file name
	std::string file_name = GetFileName(data_path_, device_name_, now_tm);
	// open file
	std::ifstream fin(file_name, std::ios::binary);
	if (!fin.good()) {
		std::cout << "[Error] Could not open file " << file_name << "\n";
		return -2;
	}
	// start position to read, the exact poisiton to avoid less than 0
	int start = now_second + 1 - seconds;
	// get offset
	size_t offset = sizeof(ScalerFileHeader)
	 	+ start * kMaxScalers * sizeof(uint32_t);
	fin.seekg(offset);
	// read value
	uint32_t read_value[kMaxScalers];
	// read loop
	for (int i = 0; i < seconds-1; ++i) {
		fin.read((char*)read_value, sizeof(uint32_t)*kMaxScalers);
		++sum_number;
		for (size_t j = 0; j < indexes.size(); ++j) {
			sum[j] += read_value[indexes[j]];
			if (sum_number == average) {
				scalers[j].push_back(std::round(sum[j] / average));
				sum[j] = 0.0;
			}
		}
		if (sum_number == average) {
			sum_number = 0;
		}
	}
	// close file
	fin.close();

	// add the current scaler value
	for (size_t i = 0; i < indexes.size(); ++i) {
		sum[i] += memory_->scaler[indexes[i]].value;
		scalers[i].push_back(std::round(sum[i] / average));
	}

	// std::cout << "\nRead recent scaler index " << index
	// 	<< ", range " << seconds << ", average " << average << "\n";
	// for (size_t i = 0; i < 6; ++i) {
	// 	for (size_t j = 0; j < 20; ++j) {
	// 		std::cout << value[i*20+j] << ", ";
	// 	}
	// 	std::cout << "\n";
	// }

	return 0;
}


/// @brief get file stream, create a new file if not exist
/// @param[in] file_name file name
/// @param[out] fout output file stream
/// @returns 0 if successful, -1 otherwise
///
int GetFileStream(const char *file_name, std::fstream &fout) {
	// check file existence
	if (access(file_name, F_OK)) {
		// open file
		fout.open(file_name, std::ios::binary | std::ios::out);
		if (!fout.good()) return -1;
		// write header
		ScalerFileHeader header;
		header.version = 1;
		header.number = kMaxScalers;
		header.reserve1 = header.reserve2 = 0;
		fout.write((char*)&header, sizeof(ScalerFileHeader));
		uint32_t scalers[kMaxScalers];
		memset(scalers, 0, sizeof(uint32_t)*kMaxScalers);
		for (size_t i = 0; i < 86400; ++i) {
			fout.write((char*)scalers, sizeof(uint32_t)*kMaxScalers);
		}
	} else {
		fout.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
		if (!fout.good()) return -1;
	}
	return 0;
}


int ScalerService::WriteScaler() const noexcept {
	// generate file name according to the date
	time_t current_time = time(NULL);
	tm *current_tm = localtime(&current_time);
	int current_hour = current_tm->tm_hour;
	int current_min = current_tm->tm_min;
	int current_sec = current_tm->tm_sec;
	size_t seconds = ((current_hour * 60 + current_min) * 60) + current_sec;

	// file name
	std::string file_name = GetFileName(data_path_, device_name_, current_tm);
	// open file
	std::fstream fout;
	if (GetFileStream(file_name.c_str(), fout)) {
		std::cout << "[Error] Open file " << file_name << " failed.\n";
		return -1;
	}

	// write data
	size_t offset = sizeof(ScalerFileHeader)
		+ seconds*kMaxScalers*sizeof(uint32_t);
	fout.seekp(offset);
	uint32_t scalers[kMaxScalers];
	for (size_t i = 0; i < kMaxScalers; ++i) {
		scalers[i] = memory_->scaler[i].value;
	}
	fout.write((char*)scalers, sizeof(uint32_t)*kMaxScalers);
	// clos file
	fout.close();

	return 0;
}


void ScalerService::Serve() noexcept {
	// server address
	std::string server_address = "0.0.0.0:" + std::to_string(port_);
	// server builder
	grpc::ServerBuilder builder;
	// listen without authentication
	builder.AddListeningPort(
		server_address, grpc::InsecureServerCredentials()
	);
	// register service
	builder.RegisterService(this);
	// assemble the server
	std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
	// shwo listening
	if (log_level_ >= kInfo) {
		std::cout << "[Info] Server listening on " << server_address << "\n";
	}
	// wait for shutdown
	server->Wait();
}


grpc::ServerUnaryReactor* ScalerService::GetState(
	grpc::CallbackServerContext* context,
	const Request*,
	Response *response
) {
	response->set_value(int(keep_running));
	auto *reactor = context->DefaultReactor();
	reactor->Finish(grpc::Status::OK);
	return reactor;
}


grpc::ServerWriteReactor<Response>* ScalerService::GetScaler(
	grpc::CallbackServerContext*,
	const Request*
) {

	class ScalerWriter : public grpc::ServerWriteReactor<Response> {
	public:
		ScalerWriter(const std::vector<Response> &responses)
		: index_(0), responses_(responses) {
			NextWrite();
		}

		virtual void OnWriteDone(bool ok) override {
			if (!ok) {
				Finish(grpc::Status(
					grpc::StatusCode::UNKNOWN, "Unexpected failure"
				));
			} else {
				NextWrite();
			}
		}

		void OnDone() override {
			delete this;
		}

	private:
		void NextWrite() {
			if (index_ < kMaxScalers) {
				const size_t index = index_;
				StartWrite(responses_.data() + index);
				index_++;
				return;
			}
			Finish(grpc::Status::OK);
		}

		size_t index_;
		std::vector<Response> responses_;
	};

	std::vector<Response> responses;
	for (size_t i = 0; i < kMaxScalers; ++i) {
		Response response;
		response.set_value(memory_->scaler[i].value);
		responses.push_back(response);
	}

	return new ScalerWriter(responses);
}


class ScalerWriter : public grpc::ServerWriteReactor<Response> {
public:
	ScalerWriter(const std::vector<Response> &responses)
	: index_(0), responses_(responses) {
		if (responses.empty()) {
			Finish(grpc::Status(
				grpc::StatusCode::DATA_LOSS, "Read data failure"
			));
		} else {
			NextWrite();
		}
	}

	virtual void OnWriteDone(bool ok) override {
		if (!ok) {
			Finish(grpc::Status(
				grpc::StatusCode::UNKNOWN, "Unexpected failure"
			));
		} else {
			NextWrite();
		}
	}

	void OnDone() override {
		delete this;
	}

private:
	void NextWrite() {
		if (index_ < responses_.size()) {
			const size_t index = index_;
			index_++;
			StartWrite(responses_.data() + index);
			return;
		}
		Finish(grpc::Status::OK);
	}

	size_t index_;
	std::vector<Response> responses_;
};


grpc::ServerWriteReactor<Response>* ScalerService::GetScalerRecent(
	grpc::CallbackServerContext*,
	const RecentRequest* request
) {
<<<<<<< HEAD
	int range, average;
=======
	int range = 120;
	int average = 1;
>>>>>>> dev
	if (request->type() == 0) {
		range = 120;
		average = 1;
	} else if (request->type() == 1) {
		range = 1200;
		average = 10;
	} else if (request->type() == 2) {
		range = 7200;
		average = 60;
	} else if (request->type() == 3) {
		range = 86400;
		average = 720;
	}
	std::vector<std::vector<uint32_t>> scalers;
	std::vector<Response> responses;
	// get recent scalers from file
	int result = ReadRecentScaler(request->flag(), range, average, scalers);
	if (result) {
		if (log_level_ >= kWarn) {
			std::cout << "[Warn] Read recent scalers from file faied, "
				<< "code: " << result << ".\n";
		}
		return new ScalerWriter(responses);
	}

	for (const auto &scaler : scalers) {
		for (const auto &value : scaler) {
			Response response;
			response.set_value(value);
			responses.push_back(response);
		}
	}

	return new ScalerWriter(responses);
}


grpc::ServerWriteReactor<Response>* ScalerService::GetScalerDate(
	grpc::CallbackServerContext*,
	const DateRequest *request
) {
	time_t t = time(NULL);
	tm *date = localtime(&t);
	date->tm_year = request->year() - 1900;
	date->tm_mon = request->month() - 1;
	date->tm_mday = request->day();
	mktime(date);

	std::vector<Response> responses;
	std::vector<std::vector<uint32_t>> scalers;
	int result = ReadDateScaler(date, request->flag(), 0, 120, 720, scalers);
	if (result) {
		if (log_level_ >= kWarn) {
			std::cout << "[Warn] Read date scaler from file failed, "
				<< "code: " << result << "\n";
		}
		return new ScalerWriter(responses);
	}

	for (const auto &scaler : scalers) {
		for (const auto &value : scaler) {
			Response response;
			response.set_value(value);
			responses.push_back(response);
		}
	}

	return new ScalerWriter(responses);
}


grpc::ServerWriteReactor<Expression>* ScalerService::GetConfig(
	grpc::CallbackServerContext*,
	const Request*
) {
	class ExpressionWriter : public grpc::ServerWriteReactor<Expression> {
	public:
		ExpressionWriter(const std::vector<Expression> &expressions)
		: expressions_(expressions), index_(0) {
			NextWrite();
		}

		void OnWriteDone(bool ok) override {
			if (!ok) Finish(
				grpc::Status(grpc::StatusCode::UNKNOWN, "Unexpected Failure")
			);
			NextWrite();
		}

		void OnDone() override {
			delete this;
		}

	private:
		void NextWrite() {
			if (index_ < expressions_.size()) {
				const size_t index = index_;
				index_++;
				StartWrite(expressions_.data()+index);
				return;
			}
			Finish(grpc::Status::OK);
		}

		std::vector<Expression> expressions_;
		size_t index_;
	};

	// expressions
	std::vector<Expression> expressions;

	// config or log path
	std::string path = std::string(getenv("HOME")) + "/.easy-config-logic";

	// get last config information
	std::ifstream last_info_file(path+"/last-config.txt");
	std::string line;
	for (int i = 0; i < 3; ++i) {
		std::getline(last_info_file, line);
	}
	last_info_file.close();

	// last config file name
	std::string file_name = line + ".txt";
	std::ifstream fin(file_name);
	if (log_level_ >= kDebug) {
		std::cout << "[Debug] Try to read " << file_name << "\n";
	}

	line = line.substr(line.find_last_of("/")+1);
	line = line.substr(0, line.find_last_of("-"));
	line[line.find_last_of("-")] = ':';
	line[line.find_last_of("-")] = ':';
	line[line.find_last_of("-")] = ' ';
	Expression config_time;
	config_time.set_value(line);
	expressions.push_back(config_time);
	if (log_level_ >= kDebug) {
		std::cout << "[Debug] Read config time "
			<< line << "\n";
	}

	while (fin.good()) {
		std::getline(fin, line);
		if (line.empty()) continue;
		Expression expr;
		expr.set_value(line);
		expressions.push_back(expr);
		if (log_level_ >= kInfo) {
			std::cout << "[Info] Read expression from file "
				<< expr.value() << "\n";
		}
	}
	fin.close();

	return new ExpressionWriter(expressions);
}


grpc::ServerReadReactor<Expression>* ScalerService::SetConfig(
	grpc::CallbackServerContext*,
	Response *response
) {
	class Recorder : public grpc::ServerReadReactor<Expression> {
	public:
		Recorder(
			Response *response,
			bool test,
			LogLevel log_level
		): response_(response), test_(test), log_level_(log_level) {
			// initialize
			response_->set_value(0);
			if (log_level_ >= kDebug) {
				std::cout << "[Debug] Start to read config.\n";
			}
			StartRead(&expression_);
		}

		void OnReadDone(bool ok) override {
			if (ok) {
				if (log_level_ >= kInfo) {
					std::cout << "[Info] Read expression from client "
						<< expression_.value() << "\n";
				}
				if (config_parser_.Parse(expression_.value())) {
					response_->set_value(-1);
					Finish(grpc::Status(
						grpc::StatusCode::INVALID_ARGUMENT,
						"Parse expression failed"
					));
				}
				StartRead(&expression_);
			} else {
				if (log_level_ >= kDebug) {
					std::cout << "[Debug] Read expressions done.\n";
				}
				// read config from parser
				memory_config_.Read(&config_parser_);
				if (!test_) {
					grpc::Status bad = grpc::Status(
						grpc::StatusCode::UNAVAILABLE, "mmap failed"
					);
					// open memory file
					int fd = open("/dev/uio0", O_RDWR);
					if (fd < 0) {
						if (log_level_ >= kDebug) {
							std::cout << "[Debug] Failed to open /dev/uio0.\n";
						}
						response_->set_value(-2);
						Finish(bad);
					}
					// lock the address space
					if (flock(fd, LOCK_EX | LOCK_NB)) {
						if (log_level_ >= kDebug) {
							std::cout << "[Debug] Failed to acquire file lock.\n";
						}
						response_->set_value(-2);
						Finish(bad);
					}
					// map memory
					void *map_addr = mmap(
						NULL,
						4096,
						PROT_READ | PROT_WRITE,
						MAP_SHARED,
						fd,
						0
					);
					if (map_addr == MAP_FAILED) {
						if (log_level_ >= kDebug) {
							std::cout << "[Debug] Failed to map.\n";
						}
						response_->set_value(-2);
						Finish(bad);
					}
					volatile uint32_t *map = (uint32_t*)map_addr;

					// write config to memory
					memory_config_.MapMemory(map);

					// clean up
					flock(fd, LOCK_UN);
					munmap(map_addr, 4096);
					close(fd);
				}

				// save backup
				std::string backup_file_name =
					config_parser_.SaveConfigInformation(true);
				// save register backup
				std::ofstream backup_file(backup_file_name+"-register.txt");
				memory_config_.Print(backup_file);
				backup_file.close();

				Finish(grpc::Status::OK);
			}
		}

		void OnDone() override {
			delete this;
		}

	private:
		Response *response_;
		bool test_;
		LogLevel log_level_;
		Expression expression_;
		MemoryConfig memory_config_;
		ConfigParser config_parser_;
	};

	return new Recorder(response, test_, log_level_);
}

}