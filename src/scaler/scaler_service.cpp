#include "scaler/scaler_service.h"

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

namespace ecl {

bool ScalerService::keep_running = true;

ScalerService::ScalerService(const ServiceOption &option) noexcept
: port_(option.port)
, log_level_(option.log_level)
, test_(option.test)
, data_path_(option.data_path)
, xillybus_lite_fd_(-1)
, memory_(nullptr) {

	keep_running = true;

	if (log_level_ >= kDebug) {
		std::cout << "[Debug] Initialize scaler service:\n"
			<< "  port " << port_ << "\n"
			<< "  data path " << data_path_ << "\n"
			<< "  log level " << kLogLevelName[log_level_] << "\n"
			<< "  test " << test_ << "\n";
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


std::string GetFileName(std::string data_path, tm *t) {
	std::stringstream file_name;
	file_name << data_path << t->tm_year+1900
		<< std::setw(2) << std::setfill('0') << t->tm_mon+1
		<< std::setw(2) << std::setfill('0') << t->tm_mday
		<< ".bin";
	return file_name.str();
}


int ScalerService::ReadRecentScaler(
	size_t index,
	int seconds,
	int average,
	std::vector<uint32_t> &value
) const noexcept {
	// initialize
	value.clear();
	if (seconds <= 0) return 0;
	if (seconds % average) return -1;
	double sum = 0;
	int sum_number = 0;
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
		std::string file_name = GetFileName(data_path_, yesterday_tm);
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
			sum += read_value[index];
			++sum_number;
			if (sum_number == average) {
				value.push_back(std::round(sum / average));
				sum_number = 0;
				sum = 0.0;
			}
		}
		// change seconds
		seconds = now_second+1;
		// close file
		fin.close();
	}

	// read today's data
	// get file name
	std::string file_name = GetFileName(data_path_, now_tm);
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
		sum += read_value[index];
		++sum_number;
		if (sum_number == average) {
			value.push_back(std::round(sum / average));
			sum_number = 0;
			sum = 0.0;
		}
	}
	// close file
	fin.close();

	// add the current scaler value
	sum += memory_->scaler[index].value;
	sum_number++;
	if (sum_number == average) {
		value.push_back(std::round(sum / average));
		sum = 0.0;
		sum_number = 0;
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
	std::string file_name = GetFileName(data_path_, current_tm);
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
	const Request* request
) {

	class ScalerWriter : public grpc::ServerWriteReactor<Response> {
	public:
		ScalerWriter(
			ScalerService *service,
			std::vector<uint32_t> scaler,
			int32_t type,
			int32_t index
		)
		: service_(service)
		, scaler_(scaler)
		, type_(type)
		, index_(index) {

			range_index_ = 0;
			range_scalers_.clear();

			if (type_ == 0) {
				index_ = 0;
				NextSingleWrite();
			} else if (type_ == 1) {
				range_ = 120;
				average_ = 1;
				NextRangeWrite();
			} else if (type_ == 2) {
				range_ = 1200;
				average_ = 10;
				NextRangeWrite();
			} else if (type_ == 3) {
				range_ = 7200;
				average_ = 60;
				NextRangeWrite();
			} else if (type_ == 4) {
				range_ = 86400;
				average_ = 720;
				NextRangeWrite();
			}
		}

		virtual void OnWriteDone(bool ok) override {
			if (!ok) {
				Finish(grpc::Status(
					grpc::StatusCode::UNKNOWN, "Unexpected failure"
				));

			} else {
				if (type_ == 0) {
					NextSingleWrite();
				} else {
					NextRangeWrite();
				}
			}
		}

		void OnDone() override {
			delete this;
		}

	private:
		void NextSingleWrite() {
			if (index_ < kMaxScalers) {
				Response *response = new Response();
				response->set_value(scaler_[index_]);
				index_++;
				StartWrite(response);
				return;
			}
			Finish(grpc::Status::OK);
		}

		void NextRangeWrite() {
			// get scaler values from file
			if (range_scalers_.empty()) {
				if (service_->ReadRecentScaler(
					index_, range_, average_, range_scalers_
				)) {
					Finish(grpc::Status(
						grpc::StatusCode::DATA_LOSS,
						"Read recent scaler failed"
					));
				}
			}
			if (range_index_ < range_scalers_.size()) {
				Response *response = new Response();
				response->set_value(range_scalers_[range_index_]);
				range_index_++;
				StartWrite(response);
				return;
			}
			Finish(grpc::Status::OK);
		}

		const ScalerService *service_;
		std::vector<uint32_t> scaler_;
		int32_t type_;
		size_t index_;
		int range_;
		int average_;
		size_t range_index_;
		std::vector<uint32_t> range_scalers_;
	};

	std::vector<uint32_t> scaler;
	for (size_t i = 0; i < kMaxScalers; ++i) {
		scaler.push_back(memory_->scaler[i].value);
	}

	return new ScalerWriter(this, scaler, request->type(), request->index());
}

}