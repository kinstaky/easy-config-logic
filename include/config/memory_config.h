#ifndef __MEMORY_CONFIG_H__
#define __MEMORY_CONFIG_H__

#include <cstdint>

#include "config/config_parser.h"

namespace ecl {

class MemoryConfig {
public:

	static const size_t kFrontIoGroupNum = 3;
	static const size_t kFrontIoGroupSize = 16;
	static const size_t kFrontIoNum = kFrontIoGroupNum * kFrontIoGroupSize;
	static const size_t kFrontIoSelectionBits = 8;

	static const size_t kMultiNum = 16;
	static const size_t kMultiThresholdBits = 8;
	static const size_t kMultiBits = kFrontIoNum + kMultiThresholdBits;

	static const size_t kOrGateNum = 16;
	static const size_t kOrGateBits = kFrontIoNum + kMultiNum;

	static const size_t kAndGateNum = 16;
	static const size_t kAndGateBits = kFrontIoNum + kMultiNum + kOrGateNum;

	static const size_t kBackEnableBits = 1;
	static const size_t kBackSelectionBits = 8;

	static const size_t kDividerNum = 8;
	static const size_t kDividerSelectionBits = 8;
	static const size_t kDivider_DivisorBits = 32;

	static const size_t kDividerGateNum = 8;
	static const size_t kOperatorTypeBits = 1;
	static const size_t kDividerGate_DivderSourceSelectionBits = 4;
	static const size_t kDividerGate_OtherSourceSelectionBits = 8;
	static const size_t kDividerGateBits = 16;

	static const size_t kScalerNum = 32;
	static const size_t kScalerSourceSelectionBits = 8;
	static const size_t kScalerClockSelectionBits = 4;


	struct Memory {

		// front io config
		uint16_t rj45_enable[kFrontIoGroupNum];
		uint16_t pl_out_enable[kFrontIoGroupNum];
		uint16_t front_input_inverse[kFrontIoGroupNum];
		uint16_t front_output_inverse[kFrontIoGroupNum];
		uint8_t front_io_source[kFrontIoNum];

		// multi config
		uint16_t multi_front_selection[kMultiNum][kFrontIoGroupNum];
		uint8_t multi_threshold[kMultiNum];

		// or gate config
		uint16_t or_front_selection[kOrGateNum][kFrontIoGroupNum];
		uint16_t or_multi_selection[kOrGateNum];

		// and gate config
		uint16_t and_front_selection[kAndGateNum][kFrontIoGroupNum];
		uint16_t and_multi_selection[kAndGateNum];
		uint16_t and_or_selection[kAndGateNum];

		// back io config
		uint8_t back_enable;
		uint8_t back_source;

		// divider config
		uint8_t divider_source[kDividerNum];
		uint32_t divider_divisor[kDividerNum];

		// divider gate config
		uint8_t divider_gate_operator_type[kDividerGateNum];
		uint8_t divider_gate_divider_source[kDividerGateNum];
		uint8_t divider_gate_other_source[kDividerGateNum];

		// scaler config
		uint8_t scaler_source[kScalerNum];
		uint8_t scaler_clock_source[kScalerNum];
	};


	/// @brief Construct a new Memory Config object
	///
	MemoryConfig() noexcept;

	/// @brief default destructor
	///
	~MemoryConfig() = default;


	/// @brief clear the memory values
	///
	void Clear() noexcept;


	/// @brief read config from logic_parser
	///
	/// @param[in] parser pointer to logic parser
	/// @returns 0 on suceess, -1 on failure
	///
	int Read(ConfigParser *parser) noexcept;


	/// @brief read config from file
	///
	/// @param[in] file name of file to read
	/// @returns 0 on success, -1 on failure
	///
	int Read(const char *file) noexcept;


	/// @brief read config from logic_parser for tester
	///
	/// @param[in] parser pointer to the logic parser
	/// @returns 0 on success, -1 on failure
	///
	int TesterRead(ConfigParser *parser) noexcept;


	/// @brief write config to file
	///
	/// @param[in] file name of file to write
	/// @returns 0 on success, -1 on failure
	///
	int Write(const char *file) const noexcept;


	/// @brief print the memory
	///
	/// @param[in] os ostream
	/// @param[in] config the MemoryConfig object to print
	/// @returns input ostream after print MemoryConfig
	///
	friend std::ostream& operator<<(std::ostream &os, const MemoryConfig &config) noexcept;


	/// @brief write register memory to the shared memory
	///
	/// @param[in] map address of mapped shared memroy
	/// @returns 0 on success, -1 on failure
	///
	int MapMemory(volatile uint32_t *map) const noexcept;



	/// @brief get the const pointer to memroy struct
	///
	/// @returns const pointer to memory struct
	///
	inline const Memory* GetMemory() const noexcept {
		return &memory_;
	}


	/// @brief convert source index from ConfigParser to memroy selection
	///
	/// @param[in] source index of source from parser
	/// @returns selection index in memroy config
	///
	uint8_t ConvertSource(size_t source) const noexcept;


	/// @brief get the rj45 enable flag
	///
	/// @param[in] index index of the rj45 port group, 0 for A0-A7, 1 for A8-A15,
	/// 	2 for B0-B7, 3 for B8-B15, 4 for C0-C7, 5 for C8-C15
	/// @returns enable flag if index is valid, 0 otherwise
	///
	uint8_t Rj45Enable(size_t index) const noexcept;

private:
	Memory memory_;

};


} 					// namespace ecl

#endif				// __MEMORY_CONFIG_H__