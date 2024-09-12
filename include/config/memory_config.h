#ifndef __MEMORY_CONFIG_H__
#define __MEMORY_CONFIG_H__

#include <cstdint>

#include "config/config_parser.h"
#include "config/memory.h"
#include "i2c.h"

namespace ecl {

class MemoryConfig {
public:
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
	/// @param[in] parser pointer to logic parser
	/// @returns 0 on suceess, -1 on failure
	///
	int Read(ConfigParser *parser) noexcept;


	/// @brief read config from file
	/// @param[in] file name of file to read
	/// @returns 0 on success, -1 on failure
	///
	int Read(const char *file) noexcept;


	/// @brief read config from logic_parser for tester
	/// @param[in] parser pointer to the logic parser
	/// @returns 0 on success, -1 on failure
	///
	int TesterRead(ConfigParser *parser) noexcept;


	/// @brief write config to file
	/// @param[in] file name of file to write
	/// @returns 0 on success, -1 on failure
	///
	int Write(const char *file) const noexcept;


	/// @brief print configuration
	/// @param[in] os ostream
	/// @param[in] print_tips print tips for each line, default is false
	///
	void Print(std::ostream &os, bool print_tips = false) const noexcept;


	/// @brief write register memory to the shared memory
	/// @param[in] map address of mapped shared memroy
	/// @returns 0 on success, -1 on failure
	///
	int MapMemory(volatile uint32_t *map) const noexcept;


	/// @brief get the const pointer to memroy struct
	/// @returns const pointer to memory struct
	///
	inline const Memory* GetMemory() const noexcept {
		return &memory_;
	}


	/// @brief call I2C chips to enable RJ45 input or output
	/// @param[in] map mapped address for FPGA
	/// @param[in] index index of RJ45 port to enable
	///
	void EnableRj45(volatile uint32_t *map, uint32_t index) const noexcept;


	/// @brief convert source index from ConfigParser to memroy selection
	/// @param[in] source index of source from parser
	/// @returns selection index in memroy config
	/// @note See the sheet2 (selection) of excel file in docs for detailed
	///
	uint8_t ConvertSource(size_t source) const noexcept;


	/// @brief get the rj45 enable flag
	/// @param[in] index index of the rj45 port group, 0 for A0-A7, 1 for A8-A15,
	/// 	2 for B0-B7, 3 for B8-B15, 4 for C0-C7, 5 for C8-C15
	/// @returns enable flag if index is valid, 0 otherwise
	///
	uint8_t Rj45Enable(size_t index) const noexcept;


	/// @brief reset FPGA with reset signal
	/// @param[in] map FPGA memory address
	/// @note This function set the reset bit for 1ms and set it back to 0.
	///		So the divider can work properly.
	///
	void Reset(volatile uint32_t *map) const noexcept;

private:
	Memory memory_;
};


} 					// namespace ecl

#endif				// __MEMORY_CONFIG_H__