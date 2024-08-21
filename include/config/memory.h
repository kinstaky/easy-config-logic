#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cstdint>

namespace ecl {

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
	uint32_t i2c;
	// adder test
	uint32_t addends[2];
	uint32_t add_sum;
	// front IO config
	uint16_t rj45_enable[3];
	uint16_t pl_out_enable[3];
	uint16_t front_input_inverse[3];
	uint16_t front_output_inverse[3];
	uint8_t front_io_source[3][16];

	// back trigger all config
	uint32_t trigger_all_out_enable;
	uint8_t back_Mask;
	uint8_t extern_ts_Mask;
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