#include <cstdint>
#include <iostream>
#include <fstream>

#include "external/cxxopts.hpp"

struct ScalerFileHeader {
	uint8_t version;
	uint8_t number;
	uint16_t reserve1;
	uint32_t reserve2;
};


int main(int argc, char **argv) {
	// input file name
	std::string input_file_name;

	// parse arguments
	cxxopts::Options args("decode", "decoder for recoreded sclaers");
	args.add_options()
		("h,help", "Print usage")
		("file", "File to decode", cxxopts::value<std::string>(), "file");
	args.parse_positional({"file"});
	args.positional_help("file");
	try {
		auto result = args.parse(argc, argv);
		if (result.count("help")) {
			std::cout << args.help() << std::endl;
            return 0;
		}
		input_file_name = result["file"].as<std::string>();
	} catch (const cxxopts::exceptions::exception &e) {
		std::cerr << "[Error] Parse failed: " << e.what() << std::endl;
		return -1;
	}


	// open input file
	std::ifstream fin(input_file_name, std::ios::binary);
	if (!fin.good()) {
		std::cout << "[Error] Open " << input_file_name << " failed.\n";
		return -1;
	}
	ScalerFileHeader header;
	fin.read((char*)&header, sizeof(ScalerFileHeader));


	// construct output file name
	std::string output_file_name = input_file_name.substr(
		0,
		input_file_name.find_first_of(".bin")
	) + ".csv";
	// open output file
	std::ofstream fout(output_file_name);
	if (!fout.good()) {
		std::cout << "[Error] Open " << output_file_name << " failed.\n";
        return -1;
	}


	// decode scalers
	uint32_t *scalers = new uint32_t[header.number];
	for (size_t second = 0; second < 86400; ++second) {
		fin.read((char*)scalers, sizeof(uint32_t)*header.number);
		for (uint8_t i = 0; i < header.number-1; ++i) {
			fout << scalers[i] << ",";
		}
		fout << scalers[header.number-1] << "\n";
	}


	// clear
	delete[] scalers;
	fout.close();
	fin.close();
	return 0;
}