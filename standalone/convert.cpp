#include <iostream>
#include <string>
#include <fstream>

#include "config/config_parser.h"
#include "config/memory_config.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Error: " << argv[0] << " needs 2 parameters." << std::endl;
		std::cout << "Usage: " << argv[0] << " [input_file] [output_file]" << std::endl;
		std::cout << "  input_file        -- input config file" << std::endl;
		std::cout << "  output_file       -- output raw config file" << std::endl;
	}

	ecl::ConfigParser parser;
	if (parser.Read(argv[1]) != 0) {
		std::cerr << "Error: Logic parser read from file " << argv[1] << " failed." << std::endl;
		return -1;
	}


	ecl::MemoryConfig config;
	if (config.Read(&parser) != 0) {
		std::cerr << "Error: Read config from logic parser failed." << std::endl;
		return -1;
	}
	if (config.Write(argv[2]) != 0) {
		std::cerr << "Error: Write config to file " << argv[2] << " failed." << std::endl;
		return -1;
	}

	return 0;
}