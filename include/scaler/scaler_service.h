#ifndef __SCALER_SERVICE_H__
#define __SCALER_SERVICE_H__

#include <unistd.h>

#include <string>
#include <thread>
#include <memory>

#include "config/memory.h"
#include "ecl.grpc.pb.h"

namespace ecl {

enum LogLevel {
	kError = 0,
	kWarn,
	kInfo,
	kDebug
};


const char* const kLogLevelName[4] = {
	"error",
	"warn",
	"info",
	"debug"
};


struct ScalerFileHeader {
	uint8_t version;
	uint8_t number;
	uint16_t reserve1;
	uint32_t reserve2;
};


struct ServiceOption {
	// gRPC port, server listen at localhost:port
	int port;
	// log level, error, warn, info, debug
	LogLevel log_level;
	// run in test mode, 0 normal mode, >0 different test mode
	int test;
	// scaler data stored path
	std::string data_path;
	// device name to distinguish different device
	std::string device_name;

	ServiceOption() {
		port = 2233;
		log_level = kWarn;
		test = 0;
		data_path = "./";
		device_name = "";
	}
};

class ScalerService final : public EasyConfigLogic::CallbackService {
public:

	/// @brief constructor
	/// @param[in] option construct options
	///
	ScalerService(const ServiceOption &option) noexcept;


	/// @brief destructor
	///
	virtual ~ScalerService() noexcept;


	/// @brief write scaler value to file
	/// @returns 0 on success, -1 on failure
	///
	int WriteScaler() const noexcept;


	/// @brief read scaler value for one date
	/// @param[in] date pointer to c style date
	/// @param[in] flag flag that scaler needs to be read
	/// @param[in] seconds seconds for start to read in this day
	/// @param[in] size number of scaler values to read
	/// @param[in] average seconds to average
	/// @param[out] scalers output scaler values
	/// @returns 0 for success, -1 for failure
	///
	int ReadDateScaler(
		tm* date,
		int32_t flag,
		size_t seconds,
		size_t size,
		int average,
		std::vector<std::vector<uint32_t>> &scalers
	) const noexcept;


	/// @brief read recent scaler values
	/// @param[in] index index of scaler to read
	/// @param[in] seconds time in seconds to read before now
	/// @param[in] average get average value from [average] numbers
	/// @param[out] value read value from file
	/// @returns 0 if successful, -1 on invalid parameters, -2 on file error
	int ReadRecentScaler(
		size_t index,
		int seconds,
		int average,
		std::vector<uint32_t> &value
	) const noexcept;


	/// @brief start gRPC server
	///
	void Serve() noexcept;


	/// @brief print scaler on screen
	///
	void PrintScaler() const noexcept;

	// ------------------------------------------------------------------------
	//                              gRPC interface
	// ------------------------------------------------------------------------

	/// @brief get state of device
	/// @param[in] context server context, handled by gRPC
	/// @param[in] request request content, keep empty now
	/// @param[out] response current state, 0 finished, 1 not config, 2 good
	/// @returns default reactor
	///
	grpc::ServerUnaryReactor* GetState(
		grpc::CallbackServerContext *context,
		const Request *request,
		Response *response
	) override;


	/// @brief get scaler value
	/// @param[in] context server context, handled by gRPC
	/// @param[in] request request content,
	///		type 0 refers to get current scaler,
	///		type 1+ refers to get range of scaler values
	/// @returns reactor to write scaler values
	///
	grpc::ServerWriteReactor<Response>* GetScaler(
		grpc::CallbackServerContext *context,
		const Request *request
	) override;


	/// @brief get scaler date value
	/// @param[in] context server context, handled by gRPC
	/// @param[in] request request content, fill date
	/// @returns reactor to write scaler values
	///
	grpc::ServerWriteReactor<Response>* GetScalerDate(
		grpc::CallbackServerContext *context,
		const DateRequest *request
	) override;


	/// @brief get last FPGA memory config expressions
	/// @param[in] context server context, handled by gPRC
	/// @param[in] request request content, empty now
	/// @returns reactor to write expressions
	///
	grpc::ServerWriteReactor<Expression>* GetConfig(
		grpc::CallbackServerContext *context,
		const Request *request
	) override;


	/// @brief set FPGA memory config
	/// @param[in] context server context, handled by gRPC
	/// @param[in] response response, config result
	/// @returns reactor to read expressions
	///
	grpc::ServerReadReactor<Expression>* SetConfig(
		grpc::CallbackServerContext *context,
		Response *response
	) override;


	// keep running until get SIGINT
	static bool keep_running;

private:

	// service options
	int port_;
	LogLevel log_level_;
	int test_;
	std::string data_path_;
	std::string device_name_;

	// mapped file
	int xillybus_lite_fd_;
	// maped memory
	Memory *memory_;

	// write scaler thread
	std::unique_ptr<std::thread> write_thread_;
	// test scaler thread
	std::unique_ptr<std::thread> test_thread_;
};

}	// namespace ecl

#endif // __SCALER_SERVICE_H__