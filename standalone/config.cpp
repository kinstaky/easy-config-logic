#include <fcntl.h>
#include <sys/mman.h>

#include <fstream>
#include <iostream>

#include "config/logic_parser.h"
#include "config/memory_config.h"

const size_t kMemorySize = 4096;

void PrintUsage(const char *name) {
	std::cout << "Usage: " << name << " [options] file\n"
		<< "Options:\n"
		<< "  -h          Print this help information.\n"
		<< "  -t          Config FPGA as a tester, only available when read logic expressions.\n"
		<< "  -l          Read logic expressions as config\n"
		<< "  -r          Read register memory as config, default\n"
		<< "  file        Path of config file."
		<< std::endl;
}


int main(int argc, char **argv) {
	if (argc < 2 || argc > 4) {
		std::cerr << "Error: Invalid parameters." << std::endl;
		PrintUsage(argv[0]);
		return -1;
	}

	bool test_flag = false;
	bool logic_flag = false;
	for (int i = 1; i < argc-1; ++i) {
		if (argv[i][0] != '-') {
			PrintUsage(argv[0]);
			return -1;
		}
		for (size_t j = 1; argv[i][j] != '\0'; ++j) {
			if (argv[i][j] == 'h') {
				PrintUsage(argv[0]);
				return 0;
			}
			if (argv[i][j] == 't') {
				test_flag = true;
			} else if (argv[i][j] == 'l') {
				logic_flag = true;
			} else if (argv[i][j] == 'r') {
				logic_flag = false;
			} else {
				PrintUsage(argv[0]);
				return -1;
			}
		}
	}
	if (argv[argc-1][0] == '-' && argv[argc-1][1] == 'h') {
		PrintUsage(argv[0]);
		return 0;
	} 


	ecc::MemoryConfig config;
	if (!logic_flag) {
		if (config.Read(argv[argc-1]) != 0) {
			std::cerr << "Error: Memory config read file " << argv[argc-1] << " failed." << std::endl;
			return -1;
		}
	} else {
		ecc::LogicParser parser;
		if (parser.Read(argv[argc-1]) != 0) {
			std::cerr << "Error: Parser read from file " << argv[argc-1] << " failed." << std::endl;
			return -1;
		}

		if (test_flag) {
			if (config.TesterRead(&parser) != 0) {
				std::cerr << "Error: TesterRead from parser failed." << std::endl;
				return -1;
			}
		} else {
			if (config.Read(&parser) != 0) {
				std::cerr << "Error: Read from parser failed." << std::endl;
				return -1;
			}
		}
	}


	std::cout << config << std::endl;

	// map memory
	int fd = open("/dev/uio0", O_RDWR);
	if (fd < 0) {
		std::cerr << "Error: Failed to open dev file /dev/uio0" << std::endl;
		return -1;
	}

	void *map_addr = mmap(NULL, kMemorySize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map_addr == MAP_FAILED) {
		std::cerr << "Error: Failed to mmap" << std::endl;
		return -1;
	}
	volatile uint32_t *map = (uint32_t*)map_addr;

	config.MapMemory(map);

	return 0;
}