#include <fcntl.h>
#include <sys/file.h>
#include <sys/mman.h>

#include <fstream>
#include <iostream>

#include "config/config_parser.h"
#include "config/memory_config.h"
#include "i2c.h"
#include "external/cxxopts.hpp"

const size_t kMemorySize = 4096;


int main(int argc, char **argv) {
	bool register_flag = false;
	bool no_map = false;
	std::string file_name;

	cxxopts::Options args("config", "config FPGA");
	args.add_options()
		("h,help", "Print usage")
		(
			"r,register", "Read register format config file",
			cxxopts::value<bool>()
		)
		(
			"n,nomap", "Do not mmap to /dev/uio0, for test",
			cxxopts::value<bool>()
		)
		(
			"file", "File to read",
			cxxopts::value<std::string>(), "file"
		);
	args.parse_positional({"file"});
	args.positional_help("file");

	try {
		auto result = args.parse(argc, argv);
		if (result.count("help")) {
			std::cout << args.help() << std::endl;
			return 0;
		}
		register_flag = result["register"].as<bool>();
		no_map = result["nomap"].as<bool>();
		if (!result.count("file")) {
			std::cerr << "[Error] Require [file] parameter.\n";
			return -1;
		}
		file_name = result["file"].as<std::string>();
	} catch (const cxxopts::exceptions::exception& e) {
		std::cerr << "[Error] Parse filaed: " << e.what() << "\n";
		return -1;
	}

	// memory config
	ecl::MemoryConfig config;
	// config parser
	ecl::ConfigParser parser;
	if (register_flag) {
		if (config.Read(file_name.c_str())) {
			std::cerr << "[Error] Memory config read file "
				<< file_name << " failed.\n";
			return -1;
		}
	} else {
		if (parser.Read(file_name.c_str())) {
			std::cerr << "[Error] Parser read from file "
				<< file_name << " failed.\n";
			return -1;
		}
		if (config.Read(&parser)) {
			std::cerr << "[Error] Failed to read from parser.\n";
			return -1;
		}

		// if (test_flag) {
		// 	if (config.TesterRead(&parser) != 0) {
		// 		std::cerr << "Error: TesterRead from parser failed.\n";
		// 		return -1;
		// 	}
		// } else {
		// 	if (config.Read(&parser) != 0) {
		// 		std::cerr << "Error: Read from parser failed.\n";
		// 		return -1;
		// 	}
		// }
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
	std::string backup_file_name = parser.SaveConfigInformation(!register_flag);
	// save register backup
	std::ofstream backup_file(backup_file_name+"-register.txt");
	config.Print(backup_file);
	backup_file.close();

	return 0;
}
