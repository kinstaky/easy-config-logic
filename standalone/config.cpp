#include <fcntl.h>
#include <sys/file.h>
#include <sys/mman.h>

#include <fstream>
#include <iostream>

#include "config/config_parser.h"
#include "config/memory_config.h"
#include "i2c.h"

const size_t kMemorySize = 4096;

/// @brief print usage
/// @param[in] name program name
///
void PrintUsage(const char *name) {
	std::cout << "Usage: " << name << " [options] file\n"
		<< "Options:\n"
		<< "  -h      Print this help information.\n"
		<< "  -t      Config FPGA as a tester, only available when read logic expressions.\n"
		<< "  -l      Read logic expressions as config, default.\n"
		<< "  -r      Read register memory as config.\n"
		<< "  -n      Do not map to /dev/uio0, used for test.\n"
		<< "  file    Path of config file.\n";
}


int main(int argc, char **argv) {
	if (argc < 2 || argc > 4) {
		std::cerr << "Error: Invalid parameters.\n";
		PrintUsage(argv[0]);
		return -1;
	}

	bool test_flag = false;
	bool logic_flag = true;
	bool no_map = false;
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
			} else if (argv[i][j] == 'n') {
				no_map = true;
			} else {
				PrintUsage(argv[0]);
				return -1;
			}
		}
	}

	// memory config
	ecl::MemoryConfig config;
	// config parser
	ecl::ConfigParser parser;
	if (!logic_flag) {
		if (config.Read(argv[argc-1]) != 0) {
			std::cerr << "Error: Memory config read file "
				<< argv[argc-1] << " failed.\n";
			return -1;
		}
	} else {
		if (parser.Read(argv[argc-1]) != 0) {
			std::cerr << "Error: Parser read from file "
				<< argv[argc-1] << " failed.\n";
			return -1;
		}

		if (test_flag) {
			if (config.TesterRead(&parser) != 0) {
				std::cerr << "Error: TesterRead from parser failed.\n";
				return -1;
			}
		} else {
			if (config.Read(&parser) != 0) {
				std::cerr << "Error: Read from parser failed.\n";
				return -1;
			}
		}
	}

	// show configuration
	config.Print(std::cout, true);

	if (!no_map) {
		// open memory file
		int fd = open("/dev/uio0", O_RDWR);
		if (fd < 0) {
			std::cerr << "Error: Failed to open dev file /dev/uio0.\n";
			return -1;
		}
		// lock the address space
		if (flock(fd, LOCK_EX | LOCK_NB)) {
			std::cerr << "Error: Failed to get the file lock on /dev/uio0.\n";
			return -1;
		}
		// map memory
		void *map_addr = mmap(
			NULL,
			kMemorySize,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			0
		);
		if (map_addr == MAP_FAILED) {
			std::cerr << "Error: Failed to mmap.\n";
			return -1;
		}
		volatile uint32_t *map = (uint32_t*)map_addr;

		// write config to memory
		config.MapMemory(map);

		// clean up
		flock(fd, LOCK_UN);
		munmap(map_addr, kMemorySize);
		close(fd);
	}

	// save backup
	std::string backup_file_name = parser.SaveConfigInformation(logic_flag);
	// save register backup
	std::ofstream backup_file(backup_file_name+"-register.txt");
	config.Print(backup_file);
	backup_file.close();

	return 0;
}
