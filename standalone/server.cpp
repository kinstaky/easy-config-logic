#include <cstring>
#include <iostream>

#include "service.h"
#include "external/cxxopts.hpp"

using namespace ecl;

int main(int argc, char **argv) {
	// initialize flags
	// test flag
	int test = 0;
	// show flag
	bool show = false;
	// service port
	int port = 2233;
	// data path
	std::string path;
	// device name
	std::string device_name;
	// log level
	LogLevel log_level = kWarn;

	cxxopts::Options args("server", "server for easy-config-logic");
	args.add_options()
		("h,help", "Print usage")
		("s,show", "Show scaler on screen")
		(
			"p,port", "Set listening port",
			cxxopts::value<int>()->default_value("2233"), "port"
		)
		(
			"d,path", "Set data path",
			cxxopts::value<std::string>()->default_value("./"), "path"
		)
		(
			"n,name", "Set device name",
			cxxopts::value<std::string>()->default_value(""), "name"
		)
		(
			"l,level", "Set log level, error, warn, info, debug",
			cxxopts::value<std::string>()->default_value("warn"), "level"
		)
		(
			"t,test", "Set test mode",
			cxxopts::value<int>()->default_value("0"), "mode"
		);

	try {
		auto result = args.parse(argc, argv);
		if (result.count("help")) {
			std::cout << args.help() << std::endl;
			return 0;
		}
		test = result["test"].as<int>();
		show = result["show"].as<bool>();
		port = result["port"].as<int>();
		path = result["path"].as<std::string>();
		device_name = result["name"].as<std::string>();
		std::string level_name = result["level"].as<std::string>();
		for (int i = 0; i < 4; ++i) {
			if (strcmp(level_name.c_str(), kLogLevelName[i])) continue;
			log_level = LogLevel(i);
			break;
		}
	} catch (const cxxopts::exceptions::exception &e) {
		std::cout << "[Error] Parse failed: " << e.what() << "\n";
		return -1;
	}

	ServiceOption option;
	option.test = test;
	option.log_level = log_level;
	option.data_path = path;
	option.device_name = device_name;

	if (show) {
		option.port = -1;
		ScalerService service(option);
		service.PrintScaler();
	} else {
		option.port = port;
		ScalerService service(option);
		service.Serve();
	}

	return 0;
}