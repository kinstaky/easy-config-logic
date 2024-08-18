#include "config/memory_config.h"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace ecl {

MemoryConfig::MemoryConfig() noexcept {
}


void MemoryConfig::Clear() noexcept {
	memset(&memory_, 0, sizeof(Memory));
	for (size_t i = 0; i < kMaxClocks; ++i) {
		memory_.divider_source[i] = ConvertSource(kPrimaryClockOffset);
	}
	for (size_t i = 0; i < kDividerNum; ++i) {
		memory_.divider_divisor[i] = 1u;
	}
}


int MemoryConfig::Read(ConfigParser *parser) noexcept {
	Clear();

	// read front io config
	for (size_t i = 0; i < kFrontIoNum; ++i) {
		if (parser->IsFrontInput(i)) {
			if (parser->IsFrontLemo(i)) {
				memory_.rj45_enable[i/16] |= 1u << (i % 16);
			} else if (!parser->IsFrontOutput(i)) {
				memory_.front_input_inverse[i/16] |= 1u << (i % 16);
			}
		}
		if (parser->IsFrontOutput(i)) {
			memory_.rj45_enable[i/16] |= 1u << (i % 16);
			memory_.pl_out_enable[i/16] |= 1u << (i % 16);
			if (!parser->IsFrontLemo(i) && parser->IsFrontLogicOutput(i)) {
				memory_.front_output_inverse[i/16] |= 1u << (i % 16);
			}
		}
	}
	for (size_t i = 0; i < parser->FrontOutputSize(); ++i) {
		OutputInfo info = parser->FrontOutput(i);
		uint8_t selection = ConvertSource(info.source);
		if (selection == uint8_t(-1)) {
			std::cerr << "Error: Invalid front output source " << info.source << std::endl;
			return -1;
		}
		memory_.front_io_source[info.port] = selection;
	}


	// read or gate config
	for (size_t i = 0; i < parser->OrGateSize(); ++i) {
		unsigned long long  or_gate = parser->OrGate(i).to_ullong();
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			memory_.or_front_selection[i][j] = (or_gate >> (j<<4)) & 0xffff;
		}
	}

	// read and gate config
	for (size_t i = 0; i < parser->AndGateSize(); ++i) {
		unsigned long long and_gate = parser->AndGate(i).to_ullong();
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			memory_.and_front_selection[i][j] = (and_gate >> (j<<4)) & 0xffff;
		}
		memory_.and_or_selection[i] = (and_gate >> 48) & 0xffff;
	}

	// read back config
	memory_.back_enable = parser->BackEnable();
	if (parser->BackEnable()) {
		uint8_t selection = ConvertSource(parser->BackSource());
		if (selection == uint8_t(-1)) {
			std::cerr << "Error: Invalid back source " << parser->BackSource() << std::endl;
			return -1;
		}
		memory_.back_source = selection;
	}

	// read clock config
	for (size_t i = 0; i < parser->ClockSize(); ++i) {
		size_t frequency = parser->ClockFrequency(i);
		uint8_t selection = ConvertSource(kPrimaryClockOffset);
		if (selection == uint8_t(-1)) {
			std::cerr << "Error: Invalid clock source " << kPrimaryClockOffset << std::endl;
			return -1;
		}
		memory_.divider_source[i] = selection;
		memory_.divider_divisor[i] = 100'000'000ul / frequency;
	}

	// read divider config
	// for (size_t i = 0; i < parser->DividerSize(); ++i) {
	// 	DividerInfo divider = parser->Divider(i);
	// 	uint8_t selection = ConvertSource(divider.source);
	// 	if (selection == uint8_t(-1)) {
	// 		std::cerr << "Error: Invalid divider source " << divider.source << std::endl;
	// 		return -1;
	// 	}
	// 	memory_.divider_source[divider.port+4] = selection;
	// 	memory_.divider_divisor[divider.port+4] = divider.divisor;
	// }

	// read divider gate config
	// for (size_t i = 0; i < parser->DividerGateSize(); ++i) {
	// 	DividerGateInfo info = parser->DividerGate(i);
	// 	memory_.divider_gate_operator_type[i] = info.op_type == kOperatorOr ? 0 : 1;
	// 	memory_.divider_gate_divider_source[i] = (info.divider + 4) & 0xf;

	// 	uint8_t selection = ConvertSource(info.source);
	// 	if (selection == uint8_t(-1)) {
	// 		std::cerr << "Error: Invalid divider gate other source " << info.divider+kDividersOffset << std::endl;
	// 		return -1;
	// 	}
	// 	memory_.divider_gate_other_source[i] = selection;
	// }

	// read scaler config
	for (size_t i = 0; i < parser->ScalerSize(); ++i) {
		OutputInfo info = parser->Scaler(i);
		uint8_t selection = ConvertSource(info.source);
		if (selection == uint8_t(-1)) {
			std::cerr << "Error: Invalid scaler source " << info.source << std::endl;
			return -1;
		}
		memory_.scaler_source[info.port] = selection;
		memory_.scaler_clock_source[info.port] = parser->SecondClock() - kClocksOffset;
	}

	return 0;
}




int MemoryConfig::Read(const char* file_name) noexcept {
	Clear();

	FILE *file = fopen(file_name, "r");
	if (!file) {
		std::cerr << "Error: Open file " << file_name << " failed." << std::endl;
		return -1;
	}

	// read front output config
	// read rj45 output enable
	for (size_t i = 0; i < kFrontIoGroupNum; ++i) {
		if (fscanf(file, "%hx", memory_.rj45_enable+i) != 1) {
			std::cerr << "Error: Expected 16 bits rj45_enable " << i << std::endl;
			return -1;
		}
	}

	// read pl output enable
	for (size_t i = 0; i < kFrontIoGroupNum; ++i) {
		if (fscanf(file, "%hx", memory_.pl_out_enable+i) != 1) {
			std::cerr << "Error: Expected 16 bits pl_out_enable " << i << std::endl;
			return -1;
		}
	}

	// read front input inverse
	for (size_t i = 0; i < kFrontIoGroupNum; ++i) {
		if (fscanf(file, "%hx", memory_.front_input_inverse+i) != 1) {
			std::cerr << "Error: Expected 16 bits front_input_inverse " << i << std::endl;
			return -1;
		}
	}

	// read front output inverse
	for (size_t i = 0; i < kFrontIoGroupNum; ++i) {
		if (fscanf(file, "%hx", memory_.front_output_inverse+i) != 1) {
			std::cerr << "Error: Expected 16 bits front_output_inverse " << i << std::endl;
			return -1;
		}
	}


	// read front output source
	for (size_t i = 0; i < kFrontIoNum; ++i) {
		if (fscanf(file, "%hhx", memory_.front_io_source+i) != 1) {
			std::cerr << "Error: Expected 8 bits front_io_source " << i << std::endl;
			return -1;
		}
	}

	// read multi config
	for (size_t i = 0; i < kMultiNum; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			if (fscanf(file, "%hx", memory_.multi_front_selection[i]+j) != 1) {
				std::cerr << "Error: Expected 16 bits multi_front_selection " << i << " of group " << j << std::endl;
				return -1;
			}
		}
		if (fscanf(file, "%hhu", memory_.multi_threshold+i) != 1) {
			std::cerr << "Error: Expected 8 bits multi_threshold " << i << std::endl;
			return -1;
		}
	}

	// read or gate config
	for (size_t i = 0; i < kOrGateNum; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			if (fscanf(file, "%hx", memory_.or_front_selection[i]+j) != 1) {
				std::cerr << "Error: Expected 16 bits or_front_selection " << i << " of group " << j << std::endl;
				return -1;
			}
		}
		if (fscanf(file, "%hx", memory_.or_multi_selection+i) != 1) {
			std::cerr << "Error: Expected 16 bits or_multi_selection " << i << std::endl;
			return -1;
		}
	}

	// read and gate config
	for (size_t i = 0; i < kAndGateNum; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			if (fscanf(file, "%hx", memory_.and_front_selection[i]+j) != 1) {
				std::cerr << "Error: Expected 16 bits and_front_selection " << i << " of group " << j << std::endl;
				return -1;
			}
		}
		if (fscanf(file, "%hx", memory_.and_multi_selection+i) != 1) {
			std::cerr << "Error: Expected 16 bits and_multi_selection " << i << std::endl;
			return -1;
		}
		if (fscanf(file, "%hx", memory_.and_or_selection+i) != 1) {
			std::cerr << "Error: Expected 16 bits and_or_selection and_or_selection " << i << std::endl;
			return -1;
		}
	}

	// read back config
	if (fscanf(file, "%hhx %hhx", &(memory_.back_enable), &(memory_.back_source)) != 2) {
		std::cerr << "Error: Expected 1 bit back_enable and 8 bits back_source" << std::endl;
		return -1;
	}

	// read divider config
	for (size_t i = 0; i < kDividerNum; ++i) {
		if (fscanf(file, "%hhx %u", memory_.divider_source+i, memory_.divider_divisor+i) != 2) {
			std::cerr << "Error: Expected 8 bits divider_source and 32 bits divider_divisor " << i << std::endl;
			return -1;
		}
	}

	// read divider gate config
	for (size_t i = 0; i < kDividerGateNum; ++i) {
		if (fscanf(file, "%hhx", memory_.divider_gate_operator_type+i) != 1) {
			std::cerr << "Error: Expected 1 bit divider_gate_operator_type " << i << std::endl;
			return -1;
		}
		memory_.divider_gate_operator_type[i] &= 0x1;

		if (fscanf(file, "%hhx", memory_.divider_gate_divider_source+i) != 1) {
			std::cerr << "Error: Expected 4 bits divider_gate_divider_source " << i << std::endl;
			return -1;
		}
		memory_.divider_gate_divider_source[i] &= 0xf;

		if (fscanf(file, "%hhx", memory_.divider_gate_other_source+i) != 1) {
			std::cerr << "Error: Expected 8 bits divider_gate_other_source " << i << std::endl;
			return -1;
		}
	}

	// read scaler config
	for (size_t i = 0; i < kScalerNum; ++i) {
		if (fscanf(file, "%hhx %hhx", memory_.scaler_source+i, memory_.scaler_clock_source+i) != 2) {
			std::cerr << "Error: Expected 8 bits scaler_source and 4 bits scaler_clock_source " << i << std::endl;
		}
		memory_.scaler_clock_source[i] &= 0xf;
	}

	fclose(file);
	return 0;
}


int MemoryConfig::TesterRead(ConfigParser *parser) noexcept {
	Clear();

	// read front io config
	for (size_t i = 0; i < kFrontIoNum; ++i) {
		if (parser->IsFrontInput(i)) {
			memory_.front_io_source[i] = uint8_t(0xffff);
			memory_.pl_out_enable[i/16] = 1u << (i % 16);
			memory_.rj45_enable[i/16] = 1u << (i % 16);
			if (!parser->IsFrontLemo(i) && !parser->IsFrontOutput(i)) {
				memory_.front_output_inverse[i/16] |= 1u << (i % 16);
			}
		}
		if (parser->IsFrontOutput(i)) {
			if (!parser->IsFrontLemo(i) && parser->IsFrontLogicOutput(i)) {
				memory_.front_input_inverse[i/16] |= 1u << (i % 16);
			}
		}
	}
	return 0;
}



int MemoryConfig::Write(const char *file_name) const noexcept {
	FILE *file = fopen(file_name, "w");
	if (!file) {
		std::cerr << "Error: Open file " << file_name << " failed." << std::endl;
		return -1;
	}

	// front io output
	// rj45 output enable
	for (size_t i = 0; i < kFrontIoGroupNum-1; ++i) {
		fprintf(file, "0x%04x ", memory_.rj45_enable[i]);
	}
	fprintf(file, "0x%04x\n", memory_.rj45_enable[kFrontIoGroupNum-1]);

	// pl output enable
	for (size_t i = 0; i < kFrontIoGroupNum-1; ++i) {
		fprintf(file, "0x%04x ", memory_.pl_out_enable[i]);
	}
	fprintf(file, "0x%04x\n", memory_.pl_out_enable[kFrontIoGroupNum-1]);

	// front input inverse
	for (size_t i = 0; i < kFrontIoGroupNum-1; ++i) {
		fprintf(file, "0x%04x ", memory_.front_input_inverse[i]);
	}
	fprintf(file, "0x%04x\n", memory_.front_input_inverse[kFrontIoGroupNum-1]);

	// front output inverse
	for (size_t i = 0; i < kFrontIoGroupNum-1; ++i) {
		fprintf(file, "0x%04x ", memory_.front_output_inverse[i]);
	}
	fprintf(file, "0x%04x\n\n", memory_.front_output_inverse[kFrontIoGroupNum-1]);

	// front output source
	for (size_t i = 0; i < kFrontIoNum; i += 4) {
		for (size_t j = 0; j < 3; ++j) {
			fprintf(file, "0x%02x ", memory_.front_io_source[i+j]);
		}
		fprintf(file, "0x%02x\n", memory_.front_io_source[i+3]);
		if ((i - 12) % 16 == 0)  fprintf(file, "\n");
	}

	// multi output
	for (size_t i = 0; i < kMultiNum; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			fprintf(file, "0x%04x ", memory_.multi_front_selection[i][j]);
		}
		fprintf(file, "%u\n", memory_.multi_threshold[i]);
	}
	fprintf(file, "\n");

	// or output
	for (size_t i = 0; i < kOrGateNum; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			fprintf(file, "0x%04x ", memory_.or_front_selection[i][j]);
		}
		fprintf(file, "0x%04x\n", memory_.or_multi_selection[i]);
	}
	fprintf(file, "\n");

	// and output
	for (size_t i = 0; i < kAndGateNum; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			fprintf(file, "0x%04x ", memory_.and_front_selection[i][j]);
		}
		fprintf(file, "0x%04x 0x%04x\n",
			memory_.and_multi_selection[i],
			memory_.and_or_selection[i]
		);
	}
	fprintf(file, "\n");

	// back output
	fprintf(file, "%1x 0x%02x\n\n", memory_.back_enable, memory_.back_source);

	// divider output
	for (size_t i = 0; i < kDividerNum; ++i) {
		fprintf(file, "0x%02x %u\n",
			memory_.divider_source[i],
			memory_.divider_divisor[i]
		);
	}
	fprintf(file, "\n");

	// divider gate output
	for (size_t i = 0; i < kDividerGateNum; i+=2) {
		fprintf(file, "%u 0x%1x 0x%02x %u 0x%1x 0x%02x\n",
			memory_.divider_gate_operator_type[i],
			memory_.divider_gate_divider_source[i],
			memory_.divider_gate_other_source[i],
			memory_.divider_gate_operator_type[i+1],
			memory_.divider_gate_divider_source[i+1],
			memory_.divider_gate_other_source[i+1]
		);
	}
	fprintf(file, "\n");

	// scaler output
	for (size_t i = 0; i < kScalerNum; i+=2) {
		fprintf(file, "0x%02x 0x%1x 0x%02x 0x%1x\n",
			memory_.scaler_source[i],
			memory_.scaler_clock_source[i],
			memory_.scaler_source[i+1],
			memory_.scaler_clock_source[i+1]
		);
	}


	fclose(file);
	return 0;
}


uint8_t MemoryConfig::ConvertSource(size_t source) const noexcept {
	if (source < kOrGatesOffset) {
		// front io
		return uint8_t(source);
	} else if (source < kAndGatesOffset) {
		// or gates
		return uint8_t(source - kOrGatesOffset + 64);
	} else if (source < kClocksOffset) {
		// and gates
		return uint8_t(source - kAndGatesOffset + 80);
	} else if (source < kScalersOffset) {
		// clocks
		return uint8_t(source - kClocksOffset + 96);
	} else if (source < kBackOffset) {
		// scalers
		std::cerr << "Error: Source can't be scaler." << std::endl;
		return uint8_t(-1);
		// return uint8_t(source - kScalersOffset + 112);
	} else if (source < kDividersOffset) {
		// back io
		std::cerr << "Error: Source can't be back io port." << std::endl;
		return uint8_t(-1);
	// } else if (source < kDividerGatesOffset) {
	// 	// dividers
	// 	return uint8_t(source - kDividersOffset + 100);
	// } else if (source < kPrimaryClockOffset) {
	// 	// divider gates
	// 	return uint8_t(source - kDividerGatesOffset + 104);
	} else if (source == kPrimaryClockOffset) {
		// primary clock
		return uint8_t(112);
	}

	std::cerr << "Error: Undefined source from ConfigParser " << source << std::endl;
	return uint8_t(-1);
}


std::ostream& operator<<(std::ostream &os, const MemoryConfig &config) noexcept {
	const MemoryConfig::Memory &memory = config.memory_;
	os << std::hex << std::setfill('0');

	// rj45_enable
	os << "0x" << std::setw(4) << memory.rj45_enable[0]
		<< " 0x" << std::setw(4) << memory.rj45_enable[1]
		<< " 0x" << std::setw(4) << memory.rj45_enable[2]
		<< std::string(18, ' ') << "rj45 enable A, B, C"
		<< std::endl;

	// pl_out_enable
	os << "0x" << std::setw(4) << memory.pl_out_enable[0]
		<< " 0x" << std::setw(4) << memory.pl_out_enable[1]
		<< " 0x" << std::setw(4) << memory.pl_out_enable[2]
		<< std::string(18, ' ') << "pl out enable A, B, C"
		<< std::endl;

	// front_input_inverse
	os << "0x" << std::setw(4) << memory.front_input_inverse[0]
		<< " 0x" << std::setw(4) << memory.front_input_inverse[1]
		<< " 0x" << std::setw(4) << memory.front_input_inverse[2]
		<< std::string(18, ' ') << "front input inverse A, B, C"
		<< std::endl;

	// front_output_inverse
	os << "0x" << std::setw(4) << memory.front_output_inverse[0]
		<< " 0x" << std::setw(4) << memory.front_output_inverse[1]
		<< " 0x" << std::setw(4) << memory.front_output_inverse[2]
		<< std::string(18, ' ') << "front output inverse A, B, C"
		<< std::endl;
	os << std::endl;

	// front_io_source
	for (size_t i = 0; i < kFrontIoNum; i += 4) {
		os << "0x" << std::setw(2) << int(memory.front_io_source[i])
			<< " 0x" << std::setw(2) << int(memory.front_io_source[i+1])
			<< " 0x" << std::setw(2) << int(memory.front_io_source[i+2])
			<< " 0x" << std::setw(2) << int(memory.front_io_source[i+3])
			<< std::string(19, ' ')
			<< "front io source "
			<< char('A'+i/16) << std::dec << i%16 << "-"
			<< char('A'+i/16) << (i+3)%16 << std::hex
			<< std::endl;
	}
	os << std::endl;

	// multi gate
	for (size_t i = 0; i < MemoryConfig::kMultiNum; ++i) {
		os << "0x" << std::setw(4) << memory.multi_front_selection[i][0]
			<< " 0x" << std::setw(4) << memory.multi_front_selection[i][1]
			<< " 0x" << std::setw(4) << memory.multi_front_selection[i][2]
			<< " " << std::dec << int(memory.multi_threshold[i]) << std::hex;

		std::stringstream ss;
		ss << memory.multi_threshold[i];
		os << std::string(17-ss.str().length(), ' ')
			<< "multi " << std::dec << i << std::hex
			<< " front A, B, C selections and threshold"
			<< std::endl;
	}
	os << std::endl;

	// or gate
	for (size_t i = 0; i < MemoryConfig::kOrGateNum; ++i) {
		os << "0x" << std::setw(4) << memory.or_front_selection[i][0]
			<< " 0x" << std::setw(4) << memory.or_front_selection[i][1]
			<< " 0x" << std::setw(4) << memory.or_front_selection[i][2]
			<< " 0x" << std::setw(4) << memory.or_multi_selection[i]
			<< std::string(11, ' ')
			<< "or gate " << std::dec << i << std::hex
			<< " front A, B, C and multi selections"
			<< std::endl;
	}
	os << std::endl;

	// and gate
	for (size_t i = 0; i < MemoryConfig::kAndGateNum; ++i) {
		os << "0x" << std::setw(4) << memory.and_front_selection[i][0]
			<< " 0x" << std::setw(4) << memory.and_front_selection[i][1]
			<< " 0x" << std::setw(4) << memory.and_front_selection[i][2]
			<< " 0x" << std::setw(4) << memory.and_multi_selection[i]
			<< " 0x" << std::setw(4) << memory.and_or_selection[i]
			<< std::string(4, ' ')
			<< "and gate " << std::dec << i << std::hex
			<< " front A, B, C, multi and or gates selections"
			<< std::endl;
	}
	os << std::endl;

	// back
	os << int(memory.back_enable)
		<< " 0x" << std::setw(2) << int(memory.back_source)
		<< std::string(32, ' ') << "back enable and source"
		<< std::endl;
	os << std::endl;

	// divider
	for (size_t i = 0; i < MemoryConfig::kDividerNum; ++i) {
		os << "0x" << std::setw(2) << int(memory.divider_source[i]) << " "
			<< std::dec << memory.divider_divisor[i] << std::hex;
		std::stringstream ss;
		ss << memory.divider_divisor[i];
		os << std::string(33-ss.str().length(), ' ') << "divider " << i
			<< " source selection and divisor" << std::endl;
	}
	os << std::endl;

	// divider gate
	for (size_t i = 0; i < MemoryConfig::kDividerGateNum; i += 2) {
		os << std::dec
			<< int(memory.divider_gate_operator_type[i])
			<< std::hex
			<< " 0x" << std::setw(1) << int(memory.divider_gate_divider_source[i])
			<< " 0x" << std::setw(2) << int(memory.divider_gate_other_source[i])
			<< std::dec
			<< " " << int(memory.divider_gate_operator_type[i+1])
			<< std::hex
			<< " 0x" << std::setw(1) << int(memory.divider_gate_divider_source[i+1])
			<< " 0x" << std::setw(2) << int(memory.divider_gate_other_source[i+1])
			<< std::string(17, ' ')
			<< "divider gate " << std::dec << i << ", " << i+1 << std::hex
			<< " operators, sources and the other sources"
			<< std::endl;
	}
	os << std::endl;

	// scaler
	for (size_t i = 0; i < MemoryConfig::kScalerNum; i += 2) {
		os << "0x" << std::setw(2) << int(memory.scaler_source[i])
			<< " 0x" << std::setw(1) << int(memory.scaler_clock_source[i])
			<< " 0x" << std::setw(2) << int(memory.scaler_source[i+1])
			<< " 0x" << std::setw(1) << int(memory.scaler_clock_source[i+1])
			<< std::string(21, ' ')
			<< "scaler " << std::dec << i << ", " << i+1 << std::hex
			<< " sources and clock sources"
			<< std::endl;
	}

	return os;
}



int MemoryConfig::MapMemory(volatile uint32_t *map) const noexcept {
	// rj45_enable and pl_out_enable
	for (size_t i = 0; i < kFrontIoGroupNum; ++i) {
		map[4+i] = uint32_t(memory_.rj45_enable[i])
			| (uint32_t(memory_.pl_out_enable[i]) << 16);
	}

	// input and output inverse
	for (size_t i = 0; i < kFrontIoGroupNum; ++i) {
		map[7+i] = uint32_t(memory_.front_input_inverse[i])
			| (uint32_t(memory_.front_output_inverse[i]) << 16);
	}

	// front output selections
	for (size_t i = 0; i < kFrontIoNum; i += 4) {
		map[10+i/4] = uint32_t(memory_.front_io_source[i])
			| (uint32_t(memory_.front_io_source[i+1]) << 8)
			| (uint32_t(memory_.front_io_source[i+2]) << 16)
			| (uint32_t(memory_.front_io_source[i+3]) << 24);
	}

	// multi gate selections
	for (size_t i = 0; i < kMultiNum; ++i) {
		map[25+i*2] = uint32_t(memory_.multi_front_selection[i][0])
			| (uint32_t(memory_.multi_front_selection[i][1]) << 16);
		map[26+i*2] = uint32_t(memory_.multi_front_selection[i][2])
			| ((uint32_t(memory_.multi_threshold[i]) & 0xff) << 16);
	}

	// or gate selections
	for (size_t i = 0; i < kOrGateNum; ++i) {
		map[58+i*2] = uint32_t(memory_.or_front_selection[i][0])
			| (uint32_t(memory_.or_front_selection[i][1]) << 16);
		map[59+i*2] = uint32_t(memory_.or_front_selection[i][2])
			| (uint32_t(memory_.or_multi_selection[i]) << 16);
	}

	// and gate selections
	for (size_t i = 0; i < kAndGateNum; ++i) {
		map[91+i*3] = uint32_t(memory_.and_front_selection[i][0])
			| (uint32_t(memory_.and_front_selection[i][1]) << 16);
		map[92+i*3] = uint32_t(memory_.and_front_selection[i][2])
			| (uint32_t(memory_.and_multi_selection[i]) << 16);
		map[93+i*3] = uint32_t(memory_.and_or_selection[i]) & 0xffff;
	}

	// back
	map[140] = (uint32_t(memory_.back_enable) & 0x1)
		| ((uint32_t(memory_.back_source) & 0xff) << 8);

	// divider
	for (size_t i = 0; i < kDividerNum; i += 4) {
		map[141+i/4] = uint32_t(memory_.divider_source[i])
			| (uint32_t(memory_.divider_source[i+1]) << 8)
			| (uint32_t(memory_.divider_source[i+2]) << 16)
			| (uint32_t(memory_.divider_source[i+3]) << 24);
	}
	for (size_t i = 0; i < kDividerNum; ++i) {
		map[143+i] = memory_.divider_divisor[i];
	}

	// divider gate
	for (size_t i = 0; i < kDividerGateNum; i += 2) {
		map[151+i/2] = (uint32_t(memory_.divider_gate_operator_type[i]) & 0x1)
			| ((uint32_t(memory_.divider_gate_divider_source[i]) & 0xf) << 4)
			| ((uint32_t(memory_.divider_gate_other_source[i]) & 0xff) << 8)
			| ((uint32_t(memory_.divider_gate_operator_type[i+1]) & 0x1) << 16)
			| ((uint32_t(memory_.divider_gate_divider_source[i+1]) & 0xf) << 20)
			| ((uint32_t(memory_.divider_gate_other_source[i+1]) & 0xff) << 24);
	}

	// scaler
	for (size_t i = 0; i < kScalerNum; ++i) {
		map[156+i] = uint32_t(memory_.scaler_source[i])
			| ((uint32_t(memory_.scaler_clock_source[i]) & 0xf) << 8);
	}

	return 0;
}


uint8_t MemoryConfig::Rj45Enable(size_t index) const noexcept {
	if (index > 5) {
		return 0;
	}
	return uint8_t(memory_.rj45_enable[index/2] >> (index % 2 * 8));
}


}				// namespace ecl
