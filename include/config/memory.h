#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cstdint>
#include <cstddef>

namespace ecl {

const size_t kFrontIoGroupNum = 3;
const size_t kFrontIoGroupSize = 16;
const size_t kFrontIoNum = 48;

const size_t kMaxMultiGates = 16;

const size_t kOrGatesOffset = kFrontIoNum;
const size_t kOrBits = kFrontIoNum;
const size_t kMaxOrGates = 16;

const size_t kAndGatesOffset = kOrGatesOffset + kMaxOrGates;
const size_t kAndBits = kOrBits + kMaxOrGates;
const size_t kMaxAndGates = 16;

const size_t kDividersOffset = kAndGatesOffset + kMaxAndGates;
const size_t kMaxDividers = 8;

const size_t kDividerOrGatesOffset = kDividersOffset + kMaxDividers;
const size_t kDividerOrBits = kAndBits + kMaxAndGates + kMaxDividers;
const size_t kMaxDividerOrGates = 8;

const size_t kDividerAndGatesOffset = kDividerOrGatesOffset + kMaxDividerOrGates;
const size_t kDividerAndBits = kDividerOrBits + kMaxDividerOrGates;
const size_t kMaxDividerAndGates = 8;

const size_t kClocksOffset = kDividerAndGatesOffset + kMaxDividerAndGates;
const size_t kMaxClocks = 4;

const size_t kInternalClocksOffset = kClocksOffset + kMaxClocks;
const size_t kMaxInternalClocks = 4;
const size_t kExternalClockOffset = kInternalClocksOffset + kMaxInternalClocks;
const size_t kBackOffset = kExternalClockOffset + 1;
const size_t kZeroValueOffset = kBackOffset + 1;

const size_t kMaxScalers = 32;
const size_t kScalersOffset = 255 - kMaxScalers;

const size_t kOrGateWidth = (kOrBits + 63) / 64;
const size_t kAndGateWidth = (kAndBits + 63) / 64;
const size_t kDividerOrGateWidth = (kDividerOrBits + 63) / 64;
const size_t kDividerAndGateWidth = (kDividerAndBits + 63) / 64;


struct I2C {
	uint32_t sda : 1;
	uint32_t scl : 1;
	uint32_t enable : 1;
	uint32_t reset : 1;
	uint32_t reserve : 28;
};

struct MultiGateMask {
	uint16_t front[3];
	uint8_t threshold;
	uint8_t reserve;
};

struct OrGateMask {
	uint16_t front[3];
	uint16_t multi;
};

struct AndGateMask {
	uint16_t front[3];
	uint16_t multi;
	uint16_t or_gates;
	uint16_t reserve;
};

struct DividerOrGateMask {
	uint16_t front[3];
	uint16_t or_gates;
	uint16_t and_gates;
	uint8_t divider;
	uint8_t reserve;
};

struct DividerAndGateMask {
	uint16_t front[3];
	uint16_t or_gates;
	uint16_t and_gates;
	uint8_t divider;
	uint8_t divider_or;
};

struct Scaler {
	uint8_t source;
	uint8_t clock_source : 4;
	uint32_t value : 20;
};

struct Memory {
	// I2C
	I2C i2c;
	// adder test
	uint32_t addends[2];
	uint32_t add_sum;
	// front IO config
	uint16_t rj45_enable[3];
	uint16_t pl_out_enable[3];
	uint16_t front_input_inverse[3];
	uint16_t front_output_inverse[3];
	uint8_t front_io_source[48];

	// back trigger all config
	uint32_t trigger_all_out_enable;
	uint8_t back_selection;
	uint8_t extern_ts_selection;
	uint16_t back_reserve;

	// front IO value
	uint16_t front_value[3];
	uint16_t front_value_reserve;

	// multi config
	MultiGateMask multi_gates[16];
	uint16_t multi_gate_value;
	uint16_t multi_gate_reserve;

	// or gate config
	OrGateMask or_gates[16];
	uint16_t or_gate_value;
	uint16_t or_gate_reserve;

	// and gate config
	AndGateMask and_gates[16];
	uint16_t and_gate_value;
	uint16_t and_gate_reserve;

	// divider
	uint8_t divider_source[8];
	uint16_t divisor[8];

	// divider or gates
	DividerOrGateMask divider_or[8];

	// divider and gates
	DividerAndGateMask divider_and[8];

	// clock divider
	uint8_t clock_divider_source[4];
	uint32_t clock_divisor[4];

	// scaler
	Scaler scaler[32];
};

};	// namespace ecl

#endif	// __MEMORY_H__