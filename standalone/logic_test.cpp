#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>

#include <bitset>
#include <fstream>
#include <iostream>
#include <vector>

#include "config/config_parser.h"
#include "config/memory_config.h"


struct AndGateInfo {
	uint64_t ports;
	std::vector<size_t> or_gates;
};
struct OutPortsInfo {
	int type;
	size_t port;
	uint64_t one_port;
	size_t or_gate;
	size_t and_gate;
};
const int kTypeOnePort = 0;
const int kTypeOrGate = 1;
const int kTypeAndGate = 2;


int main(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Error: Program needs 2 parameters." << std::endl;
		std::cout << "Usage: " << argv[0] << " file0 file1" << std::endl;
		std::cout << "  file0        logic config file" << std::endl;
		std::cout << "  file1        test result file" << std::endl;
		return -1;
	}

	// read logic expressions
	ecl::ConfigParser parser;
	if (parser.Read(argv[1]) != 0) {
		std::cerr << "Error: Parser read file " << argv[1] << " failed." << std::endl;
		return -1;
	}
	uint64_t input_ports = 0;
	uint64_t output_ports = 0;
	size_t input_size = 0;
	size_t output_size = 0;
	for (size_t i = 0; i < 48; ++i) {
		if (parser.IsFrontInput(i) && !parser.IsFrontOutput(i)) {
			input_ports |= 1ul << i;
			++input_size;
		}
		if (parser.IsFrontOutput(i) && parser.IsFrontLogicOutput(i)) {
			output_ports |= 1ul << i;
			++output_size;
		}
	}
	std::cout << std::hex;
	std::cout << "Input ports " << input_ports << std::endl;
	std::cout << "Output ports " << output_ports << std::endl;
	std::cout << std::dec;

	// prepare for output information
	std::vector<OutPortsInfo> outputs;
	std::vector<uint64_t> or_gates;
	std::vector<AndGateInfo> and_gates;

	for (size_t i = 0; i < parser.FrontOutputSize(); ++i) {
		ecl::OutputInfo info = parser.FrontOutput(i);
		OutPortsInfo new_info;
		new_info.port = info.port;
		if (info.source < ecl::kOrGatesOffset) {
			// fornt io port
			new_info.type = kTypeOnePort;
			new_info.one_port = 1ul << info.source;

		} else if (info.source < ecl::kAndGatesOffset) {
			// or gate
			new_info.type = kTypeOrGate;
			uint64_t or_gate = parser.OrGate(info.source - ecl::kOrGatesOffset).to_ulong();
			size_t index = -1ul;
			// check existence of or gate
			for (size_t j = 0; j < or_gates.size(); ++j) {
				if (or_gates[j] == or_gate) {
					index = j;
					break;
				}
			}
			// not exist
			if (index == -1ul) {
				index = or_gates.size();
				or_gates.push_back(or_gate);
			}
			new_info.or_gate = index;

		} else if (info.source < ecl::kClocksOffset) {
			// and gate
			new_info.type = kTypeAndGate;
			AndGateInfo and_gate;
			and_gate.ports = parser.AndGate(info.source - ecl::kAndGatesOffset).to_ulong() & 0xffff'ffff'ffff;

			for (size_t j = ecl::kFrontIoNum; j < ecl::kAndBits; ++j) {
				if (parser.AndGate(info.source-ecl::kAndGatesOffset).test(j)) {
					uint64_t or_gate = parser.OrGate(j-ecl::kFrontIoNum).to_ulong();
					size_t or_gate_index = -1ul;
					// check existence of or gate
					for (size_t k = 0; k < and_gates.size(); ++k) {
						if (or_gates[k] == or_gate) {
							or_gate_index = or_gate;
							break;
						}
					}
					// not exist
					if (or_gate_index == -1ul) {
						or_gate_index = or_gates.size();
						or_gates.push_back(or_gate);
					}
					and_gate.or_gates.push_back(or_gate_index);
				}
			}

			size_t and_gate_index = -1ul;
			// check existence of and gate
			for (size_t j = 0; j < and_gates.size(); ++j) {
				if (and_gates[j].ports == and_gate.ports && and_gates[j].or_gates == and_gate.or_gates) {
					and_gate_index = j;
					break;
				}
			}
			// not exist
			if (and_gate_index == -1ul) {
				and_gate_index = and_gates.size();
				and_gates.push_back(and_gate);
			}
			new_info.and_gate = and_gate_index;

		} else {
			std::cerr << "Error: Invalid output source for test." << std::endl;
			return -1;
		}

		outputs.push_back(new_info);
	}



	// prepare data buffer
	size_t data_size = 1ul << input_size;
	uint64_t *data = new uint64_t[data_size];

	// read data from file
	std::ifstream file_test(argv[2], std::ios::binary);
	file_test.read((char*)data, sizeof(uint64_t) * data_size);
	file_test.close();

	// check results
	bool success = true;
	for (size_t i = 0; i < (1ul<<input_size); ++i) {
		uint64_t evaluate_result = 0;
		for (size_t j = 0; j < outputs.size(); ++j) {

			if (outputs[j].type == kTypeOnePort) {

				uint64_t value = ((data[i] & outputs[j].one_port) == 0) ? 0 : 1ul;
				evaluate_result |= value << outputs[j].port;

			} else if (outputs[j].type == kTypeOrGate) {

				uint64_t or_gate = or_gates[outputs[j].or_gate];
				uint64_t value = ((or_gate & data[i]) == 0) ? 0 : 1ul;
				evaluate_result |= value << outputs[j].port;

			} else if (outputs[j].type == kTypeAndGate) {

				const AndGateInfo &and_gate = and_gates[outputs[j].and_gate];
				uint64_t and_value = ((and_gate.ports & data[i]) == and_gate.ports) ? 1ul : 0;
				if (and_value) {
					for (size_t k = 0; k < and_gate.or_gates.size(); ++k) {
						uint64_t or_gate = or_gates[and_gate.or_gates[k]];
						uint64_t value = ((or_gate & data[i]) == 0) ? 0 : 1ul;
						if (!value) {
							and_value = 0;
							break;
						}
					}
				}
				evaluate_result |= and_value << outputs[j].port;

			} else {
				std::cerr << "Error: Invalid output type " << outputs[j].type << std::endl;
				delete[] data;
				return -1;
			}
		}
		uint64_t test_result = data[i] & output_ports;

		if (evaluate_result != test_result) {
			std::cerr << "Error: Evaluate and test result not equal.\n"
				<< "Input: " << std::hex << (data[i] & input_ports)
				<< "\nEvaluate: " << evaluate_result
				<< "\nTest: " << test_result
				<< std::endl;
			break;
		}
	}

	if (success) {
		std::cout << "\033[0;32m" << "SUCCESS" << "\033[0m" << std::endl;
	} else {
		std::cout << "\033[1;31m" << "FAIL" << "\033[0m" << std::endl;
	}

	delete[] data;
	return 0;
}