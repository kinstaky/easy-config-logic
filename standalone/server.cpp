#include <cstring>
#include <iostream>
#include <string_view>

#include "service.h"
#include "external/cxxopts.hpp"
#define TOML_ENABLE_FORMATTERS 0
#include "external/toml.hpp"


using namespace ecl;

LogLevel ParseLogLevel(const char* str, LogLevel old_level = LogLevel::kWarn) {
	for (int i = 0; i < 4; ++i) {
		if (strcmp(str, kLogLevelName[i])) continue;
		return LogLevel(i);
	}
	return old_level;
}

int main(int argc, char **argv) {
	// initialize flags
	// test flag
	int test = 0;
	// show flag
	bool show = false;
	// service port
	int port = 2233;
	// config file
	std::string config_file;
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
			"c,config", "Config from file",
			cxxopts::value<std::string>(), "file"
		)
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
		if (result.count("config")) {
			config_file = result["config"].as<std::string>();
		}
		test = result["test"].as<int>();
		show = result["show"].as<bool>();
		port = result["port"].as<int>();
		path = result["path"].as<std::string>();
		device_name = result["name"].as<std::string>();
		std::string level_name = result["level"].as<std::string>();
		log_level = ParseLogLevel(level_name.c_str());
	} catch (const cxxopts::exceptions::exception &e) {
		std::cout << "[Error] Parse failed: " << e.what() << "\n";
		return -1;
	}

	// read config file
	if (!config_file.empty()) {
		auto toml_data = toml::parse(config_file);
		test = toml::find_or<int>(toml_data, "test", 0);
		port = toml::find_or<int>(toml_data, "port", 2233);
		path = toml::find_or<std::string>(toml_data, "path", "./");
		device_name = toml::find_or<std::string>(toml_data, "name", "");
		std::string level_name =
			toml::find_or<std::string>(toml_data, "log_level", "warn");
		log_level = ParseLogLevel(level_name.c_str());
	}

	ServiceOption option;
	option.test = test;
	option.log_level = log_level;
	option.data_path = path;
	option.device_name = device_name;

	if (show) {
		option.port = -1;
		Service service(option);
		service.PrintScaler();
	} else {
		option.port = port;
		Service service(option);
		service.Serve();
	}

	return 0;
}