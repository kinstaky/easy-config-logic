#include <cstring>
#include <iostream>

#include "scaler/scaler_service.h"

using namespace ecl;

void PrintUsage(const char *name) {
	std::cout << "Usage: " << name
		<< " [options]\n"
		<< "Options:\n"
		<< "  -h         Print this help and exit.\n"
		<< "  -s         Print scaler value on screen.\n"
		<< "  -p port    Set the listening port, default is 2233.\n"
		<< "  -d path    Set data path, default is ./\n"
		<< "  -l level   Set log level: error, warn, info, debug,\n"
		<< "               default is warn.\n"
		<< "Examples:\n"
		<< "  scaler_server -s          # Print scaler values only.\n"
		<< "  scaler_server             # Run scaler service on default port.\n"
		<< "  scaler_server -p 2233 -l debug  # "
		<< "Run service on port 2233 and print log on debug level.\n";
	return;
}


/// @brief parse arguments
/// @param[in] argc number of arguments
/// @param[in] argv arguments
/// @param[out] help need help
/// @param[out] test run in test mode
/// @param[out] print whether to print scaler values on screen
/// @param[out] port run serveice on this port
/// @param[out] level set log level
/// @param[out] path data path
/// @returns start index of positional arguments if succes, if failed returns
///		-argc (negative argc) for miss argument behind option,
/// 	or -index (negative index) for invalid argument
///
int ParseArguments(
	int argc,
	char **argv,
	bool &help,
	int &test,
	bool &print,
	int &port,
	LogLevel &level,
	std::string &path
) {
	// initialize
	help = false;
	test = false;
	print = false;
	path = "./";
	// start index of positional arugments
	int result = 0;
	for (result = 1; result < argc; ++result) {
		// assumed that all options have read
		if (argv[result][0] != '-') break;
		// short option contains only one letter
		if (argv[result][2] != 0) return -result;
		if (argv[result][1] == 'h') {
			help = true;
			return result;
		} else if (argv[result][1] == 's') {
			// option of print scaler values
			print = true;
		} else if (argv[result][1] == 't') {
			// option of running in test mode
			// get mode in next argument
			++result;
			// miss argument behind option
			if (result == argc) return -argc;
			test = atoi(argv[result]);
			if (test == 0) test = 1;
		} else if (argv[result][1] == 'p') {
			// option of setting port
			// get port in next argument
			++result;
			// miss argument behind option
			if (result == argc) return -argc;
			port = atoi(argv[result]);
		} else if (argv[result][1] == 'l') {
			// option of setting log level
			// get log level in next argument
			++result;
			// miss argument behind option
			if (result == argc) return -argc;
			level = kWarn;
			for (int i = 0; i < 4; ++i) {
				if (strcmp(argv[result], kLogLevelName[i])) continue;
				level = LogLevel(i);
				break;
			}
		} else if (argv[result][1] == 'd') {
			// option of setting data path
			// get path in next argument
			++result;
			// miss argument behind option
			if (result == argc) return -argc;
			path = std::string(argv[result]);
		} else {
			return -result;
		}
	}
	return result;
}


int main(int argc, char **argv) {
	// initialize flags
	// help flag
	bool help = false;
	// test flag
	int test = 0;
	// print flag
	bool print = false;
	// service port
	int port = 2233;
	// log level
	LogLevel log_level = kWarn;
	// data path
	std::string path;

	// parse arguments and get start index of positional arguments
	int pos_start = ParseArguments(
		argc, argv,
		help, test, print, port, log_level, path
	);

	// need help
	if (help) {
		PrintUsage(argv[0]);
		return 0;
	}
	// no option arguments
	if (pos_start < 0) {
		if (-pos_start < argc) {
			std::cerr << "Error: Invaild option " << argv[-pos_start] << ".\n";
		} else {
			std::cerr << "Error: Option need parameter.\n";
		}
		PrintUsage(argv[0]);
		return -1;
	}

	ServiceOption option;
	option.test = test;
	option.log_level = log_level;
	option.data_path = path;

	if (print) {
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