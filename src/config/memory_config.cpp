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
		memory_.clock_divider_source[i] =
			ConvertSource(kInternalClocksOffset);
	}
	for (size_t i = 0; i < kMaxDividers; ++i) {
		memory_.divisor[i] = 1u;
	}
}


int MemoryConfig::Read(ConfigParser *parser) noexcept {
	// initialize
	Clear();

	// read front io config
	for (size_t i = 0; i < kFrontIoNum; ++i) {
		if (parser->IsFrontInput(i)) {
			if (parser->IsFrontLemo(i)) {
				// enbale output to RJ45 port
				memory_.rj45_enable[i/16] |= 1u << (i % 16);
			} else {
				// invert input signal, except for LEMO input
				memory_.front_input_inverse[i/16] |= 1u << (i % 16);
			}
		}
		if (parser->IsFrontOutput(i)) {
			// enable RJ45 output
			memory_.rj45_enable[i/16] |= 1u << (i % 16);
			// enable PL output
			memory_.pl_out_enable[i/16] |= 1u << (i % 16);
			// don't invert LEMO output and clock output
			if (!parser->IsFrontLemo(i) && parser->IsFrontLogicOutput(i)) {
				memory_.front_output_inverse[i/16] |= 1u << (i % 16);
			}
		}
	}
	// read front IO selection
	for (size_t i = 0; i < parser->FrontOutputSize(); ++i) {
		// get output information
		OutputInfo info = parser->FrontOutput(i);
		// convert source
		uint8_t selection = ConvertSource(info.source);
		// invalid selection
		if (selection == uint8_t(-1)) {
			std::cerr << "Error: Invalid front output source "
				<< info.source << ", for port " << info.port << "\n";
			return -1;
		}
		// set selection
		memory_.front_io_source[info.port] = selection;
	}

	// read back selection
	if (parser->BackEnable()) {
		// set enable
		memory_.trigger_all_out_enable |= 0x1 << 6;
		// selection
		uint8_t selection = ConvertSource(parser->BackSource());
		if (selection == uint8_t(-1)) {
			std::cerr << "Error: Invalid back source "
				<< parser->BackSource() << "\n";
			return -1;
		}
		memory_.back_selection = selection;
	}

	// read external clock
	if (parser->ExternalClockEnable()) {
		// set enable
		memory_.trigger_all_out_enable |= 0x1 << 27;
		// selection
		uint8_t selection = ConvertSource(parser->ExternalClock());
		if (selection == uint8_t(-1)) {
			std::cerr << "Error: Invalid external clock selection "
				<< parser->ExternalClock() << "\n";
			return -1;
		}
		memory_.extern_ts_selection = selection;
	}


	// read or gate config
	for (size_t i = 0; i < parser->OrGateSize(); ++i) {
		auto mask = parser->OrGate(i);
		if (!mask) {
			std::cerr << "Error: Get or gate " << i << " failed.\n";
			return -1;
		}
		for (int j = 0; j < 3; ++j) {
			memory_.or_gates[i].front[j] =
				uint16_t((mask->At(0) >> (16*j)) & 0xffff);
		}
		memory_.or_gates[i].multi = 0;
	}

	// read and gate config
	for (size_t i = 0; i < parser->AndGateSize(); ++i) {
		auto mask = parser->AndGate(i);
		if (!mask) {
			std::cerr << "Error: Get and gate " << i << " failed.\n";
			return -1;
		}
		// set front IO mask
		for (int j = 0; j < 3; ++j) {
			memory_.and_gates[i].front[j] =
				uint16_t((mask->At(0) >> (16*j)) & 0xffff);
		}
		// set multi mask
		memory_.and_gates[i].multi = 0;
		// set or gates mask
		memory_.and_gates[i].or_gates = uint16_t((mask->At(0) >> 48) & 0xffff);
	}


	// read divider config
	for (size_t i = 0; i < parser->DividerSize(); ++i) {
		// get divier information
		DividerInfo info = parser->Divider(i);
		// get divider source
		uint8_t selection = ConvertSource(info.source);
		if (selection == uint8_t(-1)) {
			std::cerr << "Error: Invalid divider source "
				<< info.source << " for divider " << i << "\n";
			return -1;
		}
		// set source
		memory_.divider_source[i] = selection;
		// set divisor
		memory_.divisor[i] = uint16_t(info.divisor);
	}


	// read divider-or gate config
	for (size_t i = 0; i < parser->DividerOrGateSize(); ++i) {
		// get divider-or gate mask
		auto mask = parser->DividerOrGate(i);
		if (!mask) {
			std::cerr << "Error: Get divider-or gate " << i << " failed.\n";
			return -1;
		}
		// set front IO mask
		for (int j = 0; j < 3; ++j) {
			memory_.divider_or[i].front[j] =
				uint16_t((mask->At(0) >> (16*j)) & 0xffff);
		}
		// set or gates mask
		memory_.divider_or[i].or_gates = uint16_t((mask->At(0) >> 48) & 0xffff);
		// set and gates mask
		memory_.divider_or[i].and_gates = uint16_t(mask->At(1) & 0xffff);
		// set divider mask
		memory_.divider_or[i].divider = uint8_t((mask->At(1) >> 16) & 0xff);
	}


	// read divider-and gate config
	for (size_t i = 0; i < parser->DividerAndGateSize(); ++i) {
		// get divider-and gate mask
		auto mask = parser->DividerAndGate(i);
		if (!mask) {
			std::cerr << "Error: Get divider-and gate " << i << " failed.\n";
			return -1;
		}
		// set front IO mask
		for (int j = 0; j < 3; ++j) {
			memory_.divider_and[i].front[j] =
				uint16_t((mask->At(0) >> (16*j)) & 0xffff);
		}
		// set or gates mask
		memory_.divider_and[i].or_gates =
			uint16_t((mask->At(0) >> 48) & 0xffff);
		// set and gates mask
		memory_.divider_and[i].and_gates = uint16_t(mask->At(1) & 0xffff);
		// set divider mask
		memory_.divider_and[i].divider = uint8_t((mask->At(1) >> 16) & 0xff);
		// set divider-or gates mask
		memory_.divider_and[i].divider_or = uint8_t((mask->At(1) >> 24) & 0xff);
	}


	// read clock config
	for (size_t i = 0; i < parser->ClockSize(); ++i) {
		size_t frequency = parser->ClockFrequency(i);
		uint8_t selection = ConvertSource(kInternalClocksOffset);
		if (selection == uint8_t(-1)) {
			std::cerr << "Error: Invalid clock source "
				<< kInternalClocksOffset << "\n";
			return -1;
		}
		memory_.clock_divider_source[i] = selection;
		memory_.clock_divisor[i] = 100'000'000ul / frequency;
	}


	// read scaler config
	for (size_t i = 0; i < parser->ScalerSize(); ++i) {
		OutputInfo info = parser->Scaler(i);
		uint8_t selection = ConvertSource(info.source);
		if (selection == uint8_t(-1)) {
			std::cerr << "Error: Invalid scaler source "
				<< info.source << " for scaler " << i << "\n";
			return -1;
		}
		memory_.scaler[info.port].source = selection;
		memory_.scaler[info.port].clock_source =
			parser->SecondClock() - kClocksOffset;
	}

	return 0;
}


int MemoryConfig::Read(const char* file_name) noexcept {
	Clear();

	FILE *file = fopen(file_name, "r");
	if (!file) {
		std::cerr << "Error: Open file "
			<< file_name << " failed.\n";
		return -1;
	}

	// read front output config
	// read rj45 output enable
	for (size_t i = 0; i < kFrontIoGroupNum; ++i) {
		if (fscanf(file, "%hx", memory_.rj45_enable+i) != 1) {
			std::cerr << "Error: Expect 16 bits rj45_enable " << i << "\n";
			return -1;
		}
	}

	// read pl output enable
	for (size_t i = 0; i < kFrontIoGroupNum; ++i) {
		if (fscanf(file, "%hx", memory_.pl_out_enable+i) != 1) {
			std::cerr << "Error: Expect 16 bits pl_out_enable " << i << "\n";
			return -1;
		}
	}

	// read front input inverse
	for (size_t i = 0; i < kFrontIoGroupNum; ++i) {
		if (fscanf(file, "%hx", memory_.front_input_inverse+i) != 1) {
			std::cerr << "Error: Expect 16 bits front_input_inverse "
				<< i << "\n";
			return -1;
		}
	}

	// read front output inverse
	for (size_t i = 0; i < kFrontIoGroupNum; ++i) {
		if (fscanf(file, "%hx", memory_.front_output_inverse+i) != 1) {
			std::cerr << "Error: Expect 16 bits front_output_inverse "
				<< i << "\n";
			return -1;
		}
	}

	// read front output source
	for (size_t i = 0; i < kFrontIoNum; ++i) {
		if (fscanf(file, "%hhu", memory_.front_io_source+i) != 1) {
			std::cerr << "Error: Expect 8 bits front_io_source "
				<< i << "\n";
			return -1;
		}
	}

	// read trigger all config
	uint16_t trigger_all_high, trigger_all_low;
	if (fscanf(file, "%hx %hx", &trigger_all_high, &trigger_all_low) != 2) {
		std::cerr << "Error: Expecte 2 16bits backc trigger selection.\n";
		return -1;
	}
	memory_.trigger_all_out_enable =
		(uint32_t(trigger_all_high) << 16) | trigger_all_low;

	// read back and external clock selection
	if (fscanf(file, "%hhu", &(memory_.back_selection)) != 1) {
		std::cerr << "Error: Expect 8 bits back selection.\n";
		return -1;
	}
	if (fscanf(file, "%hhu", &(memory_.extern_ts_selection)) != 1) {
		std::cerr << "Error: Expect 8 bits extern ts selection.\n";
		return -1;
	}

	// read multi config
	for (size_t i = 0; i < kMaxMultiGates; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			if (fscanf(file, "%hx", memory_.multi_gates[i].front+j) != 1) {
				std::cerr << "Error: Expect 16 bits multi_front_selection "
					<< i << " of group " << j << "\n";
				return -1;
			}
		}
		if (fscanf(file, "%hhu", &(memory_.multi_gates[i].threshold)) != 1) {
			std::cerr << "Error: Expect 8 bits multi_threshold "
				<< i << "\n";
			return -1;
		}
	}

	// read or gate config
	for (size_t i = 0; i < kMaxOrGates; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			if (fscanf(file, "%hx", memory_.or_gates[i].front+j) != 1) {
				std::cerr << "Error: Expect 16 bits or_front_selection "
					<< i << " of group " << j << "\n";
				return -1;
			}
		}
		if (fscanf(file, "%hx", &(memory_.or_gates[i].multi)) != 1) {
			std::cerr << "Error: Expect 16 bits or_multi_selection "
				<< i << "\n";
			return -1;
		}
	}

	// read and gate config
	for (size_t i = 0; i < kMaxAndGates; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			if (fscanf(file, "%hx", memory_.and_gates[i].front+j) != 1) {
				std::cerr << "Error: Expect 16 bits and gates front mask "
					<< i << " of group " << j << "\n";
				return -1;
			}
		}
		if (fscanf(file, "%hx", &(memory_.and_gates[i].multi)) != 1) {
			std::cerr << "Error: Expect 16 bits and gates multi mask "
				<< i << "\n";
			return -1;
		}
		if (fscanf(file, "%hx", &(memory_.and_gates[i].or_gates)) != 1) {
			std::cerr << "Error: Expect 16 bits and gates or mask "
				<< i << "\n";
			return -1;
		}
	}

	// read divider config
	for (size_t i = 0; i < kMaxDividers; ++i) {
		if (fscanf(file, "%hhu", memory_.divider_source+i) != 1) {
			std::cerr << "Error: Expect 8 bits divider_source "
				<< i << "\n";
			return -1;
		}
		if (fscanf(file, "%hu", memory_.divisor+i) != 1) {
			std::cerr << "Error: Expect 16 bits divisor " << i << "\n";
			return -1;
		}
	}

	// read divider or gate
	for (size_t i = 0; i < kMaxDividerOrGates; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			if (fscanf(file, "%hx", memory_.divider_or[i].front+j) != 1) {
				std::cerr << "Error: Expect 16 bits divider-or gates front mask "
					<< i << " of group " << j << "\n";
				return -1;
			}
		}
		if (fscanf(file, "%hx", &(memory_.divider_or[i].or_gates)) != 1) {
			std::cerr << "Error: Expect 16 bits divider-or gates or mask "
				<< i << "\n";
			return -1;
		}
		if (fscanf(file, "%hx", &(memory_.divider_or[i].and_gates)) != 1) {
			std::cerr << "Error: Expect 16 bits divider-or gates and mask "
				<< i << "\n";
			return -1;
		}
		if (fscanf(file, "%hhx", &(memory_.divider_or[i].divider)) != 1) {
			std::cerr << "Error: Expect 8 bits divider-or gates divider mask "
				<< i << "\n";
		}
	}

	// read divider and gate
	for (size_t i = 0; i < kMaxDividerAndGates; ++i) {
		for (size_t j = 0; j < kFrontIoGroupNum; ++j) {
			if (fscanf(file, "%hx", memory_.divider_and[i].front+j) != 1) {
				std::cerr << "Error: Expect 16 bits divider-and gates front mask "
					<< i << " of group " << j << "\n";
				return -1;
			}
		}
		if (fscanf(file, "%hx", &(memory_.divider_and[i].or_gates)) != 1) {
			std::cerr << "Error: Expect 16 bits divider-and gates or mask "
				<< i << "\n";
			return -1;
		}
		if (fscanf(file, "%hx", &(memory_.divider_and[i].and_gates)) != 1) {
			std::cerr << "Error: Expect 16 bits divider-and gates and mask "
				<< i << "\n";
			return -1;
		}
		if (fscanf(file, "%hhx", &(memory_.divider_and[i].divider)) != 1) {
			std::cerr << "Error: Expect 8 bits divider-and gates divider mask "
				<< i << "\n";
		}
		if (fscanf(file, "%hhx", &(memory_.divider_and[i].divider_or)) != 1) {
			std::cerr << "Error: Expect 8 bits divider-and gates divider-or mask "
				<< i << "\n";
		}
	}

	// read clock
	for (size_t i = 0; i < kMaxClocks; ++i) {
		if (fscanf(file, "%hhu", memory_.clock_divider_source+i) != 1) {
			std::cerr << "Error: Expect 8 bits clock divider source "
				<< i << "\n";
			return -1;
		}
		if (fscanf(file, "%u", memory_.clock_divisor+i) != 1) {
			std::cerr << "Error: Expect 32 bits clock divider divisor "
				<< i << "\n";
		}
	}

	// read scaler config
	for (size_t i = 0; i < kMaxScalers; ++i) {
		if (fscanf(file, "%hhu", &(memory_.scaler[i].source)) != 1) {
			std::cerr << "Error: Expect 8 bits scaler source " << i << "\n";
			return -1;
		}
		uint32_t clock_source;
		if (fscanf(file, "%u", &clock_source) != 1) {
			std::cerr << "Error: Expect 4 bits clock source " << i << "\n";
			return -1;
		}
		memory_.scaler[i].clock_source = clock_source & 0xf;
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
			if (!parser->IsFrontLemo(i)) {
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
	std::ofstream fout(file_name);
	if (!fout.good()) {
		std::cerr << "Error: Open file " << file_name << " failed.\n";
		return -1;
	}
	Print(fout, false);
	fout.close();
	return 0;
}


uint8_t MemoryConfig::ConvertSource(size_t source) const noexcept {
	if (source < kFrontIoNum) {
		// front io
		return uint8_t(source);
	} else if (source <= kOrGatesOffset+kMaxOrGates) {
		// or gates
		return uint8_t(source - kOrGatesOffset + 64);
	} else if (source <= kAndGatesOffset+kMaxAndGates) {
		// and gates
		return uint8_t(source - kAndGatesOffset + 80);
	} else if (source <= kDividersOffset+kMaxDividers) {
		// divider
		return uint8_t(source - kDividersOffset + 96);
	} else if (source <= kDividerOrGatesOffset+kMaxDividerOrGates) {
		// divider-or gates
		return uint8_t(source - kDividerOrGatesOffset + 104);
	} else if (source <= kDividerAndGatesOffset+kMaxDividerAndGates) {
		// divider-and gates
		return uint8_t(source - kDividerAndGatesOffset + 112);
	} else if (source <= kClocksOffset+kMaxClocks) {
		// clocks
		return uint8_t(source - kClocksOffset + 120);
	} else if (source <= kInternalClocksOffset+kMaxInternalClocks) {
		// primary clock
		return uint8_t(source - kInternalClocksOffset + 124);
	} else if (source == kExternalClockOffset) {
		// external clock
		return uint8_t(128);
	} else if (source == kBackOffset) {
		// signal from backplane
		return uint8_t(129);
	} else if (source == kZeroValueOffset) {
		// set value
		return uint8_t(130);
	} else if (
		source >= kScalersOffset && source <= kScalersOffset + kMaxScalers
	) {
		std::cerr << "Error: Scaler source in invalid " << source << "\n";
		return uint8_t(-1);
	}

	std::cerr << "Error: Undefined source from ConfigParser " << source << "\n";
	return uint8_t(-1);
}


void MemoryConfig::Print(std::ostream &os, bool print_tips) const noexcept {
	os << std::hex << std::setfill('0');

	// rj45_enable
	os << "0x" << std::setw(4) << memory_.rj45_enable[0]
		<< " 0x" << std::setw(4) << memory_.rj45_enable[1]
		<< " 0x" << std::setw(4) << memory_.rj45_enable[2];
	if (print_tips) {
		os << std::string(18, ' ') << "rj45 enable A, B, C";
	}
	os << "\n";

	// pl_out_enable
	os << "0x" << std::setw(4) << memory_.pl_out_enable[0]
		<< " 0x" << std::setw(4) << memory_.pl_out_enable[1]
		<< " 0x" << std::setw(4) << memory_.pl_out_enable[2];
	if (print_tips) {
		os << std::string(18, ' ') << "pl out enable A, B, C";
	}
	os << "\n";

	// front_input_inverse
	os << "0x" << std::setw(4) << memory_.front_input_inverse[0]
		<< " 0x" << std::setw(4) << memory_.front_input_inverse[1]
		<< " 0x" << std::setw(4) << memory_.front_input_inverse[2];
	if (print_tips) {
		os << std::string(18, ' ') << "front input inverse A, B, C";
	}
	os << "\n";

	// front_output_inverse
	os << "0x" << std::setw(4) << memory_.front_output_inverse[0]
		<< " 0x" << std::setw(4) << memory_.front_output_inverse[1]
		<< " 0x" << std::setw(4) << memory_.front_output_inverse[2];
	if (print_tips) {
		os << std::string(18, ' ') << "front output inverse A, B, C";
	}
	os << "\n\n";

	// front_io_source
	for (size_t i = 0; i < kFrontIoNum; i += 4) {
		os << std::dec << std::setfill(' ')
			<< std::setw(3) << int(memory_.front_io_source[i])
			<< " " << std::setw(3) << int(memory_.front_io_source[i+1])
			<< " " << std::setw(3) << int(memory_.front_io_source[i+2])
			<< " " << std::setw(3) << int(memory_.front_io_source[i+3])
			<< std::hex << std::setfill('0');
		if (print_tips) {
			os << std::string(23, ' ') << "front io source "
				<< char('A'+i/16) << std::dec << i%16 << "-"
				<< char('A'+i/16) << (i+3)%16 << std::hex;
		}
		os << "\n";
	}
	os << "\n";

	// trigger all config
	os << "0x" << std::setw(4) << (memory_.trigger_all_out_enable >> 16)
		<< " 0x" << std::setw(4) << (memory_.trigger_all_out_enable & 0xffff)
		<< std::dec << std::setfill(' ')
		<< " " << std::setw(3) << uint32_t(memory_.back_selection)
		<< " " << std::setw(3) << uint32_t(memory_.extern_ts_selection)
		<< std::hex << std::setfill('0');
	if (print_tips) {
		os << std::string(16, ' ')
			<< "back and external clock signal enable and selection";
	}
	os << "\n\n";

	// multi gate
	for (size_t i = 0; i < kMaxMultiGates; ++i) {
		os << "0x" << std::setw(4) << memory_.multi_gates[i].front[0]
			<< " 0x" << std::setw(4) << memory_.multi_gates[i].front[1]
			<< " 0x" << std::setw(4) << memory_.multi_gates[i].front[2]
			<< std::dec << std::setfill(' ')
			<< " " << std::setw(2) << int(memory_.multi_gates[i].threshold)
			<< std::hex << std::setfill('0');

		if (print_tips) {
			os << std::string(15, ' ') << "multi "
				<< std::dec << i << std::hex
				<< " front A, B, C mask and threshold";
		}
		os << "\n";
	}
	os << "\n";

	// or gate
	for (size_t i = 0; i < kMaxOrGates; ++i) {
		os << "0x" << std::setw(4) << memory_.or_gates[i].front[0]
			<< " 0x" << std::setw(4) << memory_.or_gates[i].front[1]
			<< " 0x" << std::setw(4) << memory_.or_gates[i].front[2]
			<< " 0x" << std::setw(4) << memory_.or_gates[i].multi;
		if (print_tips) {
			os << std::string(11, ' ') << "or gate "
				<< std::dec << i << std::hex
				<< " front A, B, C and multi mask";
		}
		os << "\n";
	}
	os << "\n";

	// and gate
	for (size_t i = 0; i < kMaxAndGates; ++i) {
		os << "0x" << std::setw(4) << memory_.and_gates[i].front[0]
			<< " 0x" << std::setw(4) << memory_.and_gates[i].front[1]
			<< " 0x" << std::setw(4) << memory_.and_gates[i].front[2]
			<< " 0x" << std::setw(4) << memory_.and_gates[i].multi
			<< " 0x" << std::setw(4) << memory_.and_gates[i].or_gates;
		if (print_tips) {
			os << std::string(4, ' ') << "and gate "
				<< std::dec << i << std::hex
				<< " front A, B, C, multi and or gates mask";
		}
		os << "\n";
	}
	os << "\n";

	// divider
	for (size_t i = 0; i < kMaxDividers; ++i) {
		os << std::dec << std::setfill(' ')
			<< std::setw(3) << uint32_t(memory_.divider_source[i])
			<< " " << memory_.divisor[i]
			<< std::hex << std::setfill('0');
		if (print_tips) {
			std::stringstream ss;
			ss << memory_.divisor[i];
			os << std::string(35-ss.str().length(), ' ') << "divider "
				<< i << " source selection and divisor";
		}
		os << "\n";
	}
	os << "\n";

	// divider or gate
	for (size_t i = 0; i < kMaxDividerOrGates; ++i) {
		os << "0x" << std::setw(4) << memory_.divider_or[i].front[0]
			<< " 0x" << std::setw(4) << memory_.divider_or[i].front[1]
			<< " 0x" << std::setw(4) << memory_.divider_or[i].front[2];
		if (print_tips) {
			os << std::string(18, ' ') << "divider or gate " << i
				<< " front A, B, C mask";
		}
		os << "\n";
		os << "0x" << std::setw(4) << memory_.divider_or[i].or_gates
			<< " 0x" << std::setw(4) << memory_.divider_or[i].and_gates
			<< " 0x" << std::setw(2) << int(memory_.divider_or[i].divider);
		if (print_tips) {
			os << std::string(20, ' ') << "divider or gate " << i
				<< " or gates, and gates, divider mask";
		}
		os << "\n";
	}
	os << "\n";

	// divider and gate
	for (size_t i = 0; i < kMaxDividerAndGates; ++i) {
		os << "0x" << std::setw(4) << memory_.divider_and[i].front[0]
			<< " 0x" << std::setw(4) << memory_.divider_and[i].front[1]
			<< " 0x" << std::setw(4) << memory_.divider_and[i].front[2];
		if (print_tips) {
			os << std::string(18, ' ') << "divider and gate " << i
			<< " front A, B, C mask";
		}
		os << "\n";
		os << "0x" << std::setw(4) << memory_.divider_and[i].or_gates
			<< " 0x" << std::setw(4) << memory_.divider_and[i].and_gates
			<< " 0x" << std::setw(2) << int(memory_.divider_and[i].divider)
			<< " 0x" << std::setw(2) << int(memory_.divider_and[i].divider_or);
		if (print_tips) {
			os << std::string(16, ' ') << "divider and gate " << i
				<< " or gates, and gates, divider, divider-or gate mask";
		}
		os << "\n";
	}
	os << "\n";

	// clock
	os << std::dec << std::setfill(' ');
	for (size_t i = 0; i < kMaxClocks; i+=2) {
		os << std::setw(3) << int(memory_.clock_divider_source[i])
			<< " " << memory_.clock_divisor[i]
			<< " " << std::setw(3) << int(memory_.clock_divider_source[i+1])
			<< " " << memory_.clock_divisor[i+1];
		if (print_tips) {
			std::stringstream ss;
			ss << memory_.clock_divisor[i] << memory_.clock_divisor[i+1];
			os << std::string(31-ss.str().length(), ' ') << "clock "
				<< i << ", " << i+1 << " source and divisor";
		}
		os << "\n";
	}
	os << "\n";

	// scaler
	for (size_t i = 0; i < kMaxScalers; i += 2) {
		os << std::setw(3) << int(memory_.scaler[i].source)
			<< " " << std::setw(1) << int(memory_.scaler[i].clock_source)
			<< " " << std::setw(3) << int(memory_.scaler[i+1].source)
			<< " " << std::setw(1) << int(memory_.scaler[i+1].clock_source);
		if (print_tips) {
			os << std::string(25, ' ') << "scaler " << i << ", " << i+1
				<< " sources and clock sources";
		}
		os << "\n";
	}
}



int MemoryConfig::MapMemory(volatile uint32_t *map) const noexcept {
	memcpy((void*)map, &memory_, sizeof(memory_));
	return 0;
}


uint8_t MemoryConfig::Rj45Enable(size_t index) const noexcept {
	if (index > 5) return 0;
	return uint8_t(memory_.rj45_enable[index/2] >> (index % 2 * 8));
}


}				// namespace ecl
