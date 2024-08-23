#ifndef __CONFIG_PARSER_H__
#define __CONFIG_PARSER_H__

#include <bitset>
#include <map>
#include <string>
#include <vector>

#include "config/memory.h"
#include "syntax/parser/token.h"
#include "standardize/standard_logic_downscale_tree.h"

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


struct OutputInfo {
	size_t port;			// port local index
	size_t source;			// source global index
};
struct DividerInfo {
	size_t source;			// source global index
	size_t divisor;			// divisor
};

class ConfigParser {
public:

	/// @brief constuctor
	///
	ConfigParser();


	/// @brief read logic expressions from file
	///
	/// @param[in] path path of input file
	/// @returns 0 on success, -1 on failure
	///
	int Read(const std::string &path) noexcept;


	/// @brief parse the expression and generate the standard logic tree
	///
	/// @param[in] expr the logic expression to parse
	/// @returns 0 on success, -1 on failure
	///
	int Parse(const std::string &expr) noexcept;


	/// @brief clear the varibles and go back to initial state
	///
	void Clear() noexcept;


	//-------------------------------------------------------------------------
	//						helper functions for method Parse
	//-------------------------------------------------------------------------

	/// @brief check the form of identifiers
	/// @param[in] tokens identifiers list to check
	/// @returns true if identifier in appropirate form, false otherwise
	///
	bool CheckIdentifiers(const std::vector<TokenPtr> &tokens) const noexcept;


	/// @brief check whether there is io conflict
	/// @param[in] tokens identifiers list to check
	/// @returns true if no conflict up to now, false otherwise
	///
	bool CheckIoConflict(const std::vector<TokenPtr> &tokens) const noexcept;


	/// @brief check the identifier is in FrontIo form
	/// @note FrontIO form means that the identifier matches one of the
	/// 	input/output port on MZTIO. It should be A0-A31, B0-B31 or C0-C31.
	///		Apparently, lemo io form is included in FrontIo form.
	/// 	Be aware of the first letter should be uppercase.
	/// @param[in] name the identifier name to check
	/// @returns true if in FrontIO form, false otherwise
	///
	bool IsFrontIo(const std::string &name) const noexcept;


	/// @brief check the identifier is in lemo io form
	/// @note Lemo io form means that the identifier matches one of the
	/// 	input/ouput port on daughter card of MZTIO. It should be
	///		A16-A31, B16-B31, or C116-C31. It's similar to the FrontIo form.
	/// @param[in] name the identifier name to check
	/// @returns true if in lemo io form, false otherwise
	///
	bool IsLemoIo(const std::string &name) const noexcept;


	/// @brief check whether the identifier represents the backplane port
	/// @param[in] name the identifier name to check
	/// @returns true if is backplane port, false otherwise
	///
	inline bool IsBack(const std::string &name) const noexcept {
		return name == "Back";
	}


	/// @brief check whether the identifier represents a clock
	/// @param[in] name the identifier name to check
	/// @returns true if is clock, false otherwise
	///
	bool IsClock(const std::string &name) const noexcept;


	/// @brief check whether the identifier represents a scaler
	/// @param[in] name the identifier name to check
	/// @returns true if is a scaler, false otherwise
	///
	bool IsScaler(const std::string &name) const noexcept;


	/// @brief check whether the identifier represents a divider
	/// @param[in] name the identifier name to check
	/// @returns true if is a divider, false otherwise
	///
	bool IsDivider(const std::string &name) const noexcept;


	/// @brief check whether the identifier represents the external clock
	/// @param[in] name identifier name to check
	/// @returns true if it's the external clock, false otherwise
	///
	inline bool IsExternalClock(const std::string &name) const noexcept {
		return name == "Extern";
	}


	/// @brief get the identifier index base on its name
	/// @note The identifier index affect the bitset of or-gates and and-gates.
	/// 	To be convenient, the index 0-47 is FrontIO identifiers. Port
	/// 	A0-A15 matches index 0-15, port B0-B15 matches index 16-31, port
	///		C0-C15 matches index 32-47.
	/// @param[in] id the name of the identifier
	/// @returns index of the identifier
	///
	size_t IdentifierIndex(const std::string &id) const noexcept;


	/// @brief generate and, or, divider-or, divider-and gate
	/// @param[in] tree pointer to standard-logic-downscale-tree
	/// @param[in] node pointer to current processing standard-logic-node
	/// @param[in] layer current layer, 0-identifier, 1-or gate, 2-and gate
	/// 	3-divider-or-gate, 4-divider-and-gate
	/// @param[inout] gate_info information of current layer's gates,
	///		includes list of gates, gate index offset, maximum number of gates
	/// @param[in] is_scaler whether variable in the left side is scaler
	/// @returns gate index (not negative) if successful, -1 on failure
	/// @see GatesInfo
	///
	int GenerateGate(
		const StandardLogicDownscaleTree *tree,
		const StandardLogicNode *node,
		const int layer,
		const bool is_scaler
	) noexcept;


	/// @brief generate a clock according to the identifier name
	/// @param[in] id name of identifier
	/// @returns generated clock index, or -1 on failure
	///
	int GenerateClock(const std::string &id) noexcept;


	/// @brief generate divider and get its index
	/// @param[in] tree pointer to standard-logic-downscal tree
	/// @param[in] node pointer to processing standard logic node
	/// @param[in] divisor divisor of this divider
	/// @param[in] is_scaler whether variable in the left side is scaler
	/// @returns global divider index if successful, -1 otherwise
	///
	int GenerateDivider(
		const StandardLogicDownscaleTree *tree,
		const StandardLogicNode *node,
		const size_t divisor,
		const bool is_scaler
	) noexcept;


	//// @brief generate or-gate or and-gate with divider output
	/// @param[in] tokens tokens of the divider and operator
	/// @param[in] other_source index of the the other source to operate with divider
	/// @returns genrated index, or -1 on failure
	///
	int GenerateDividerGate(
		const std::vector<TokenPtr> &tokens,
		size_t other_source
	) noexcept;


	/// @brief parse the clock name and get the frequency
	/// @param[in] clock the name of the clock identifier
	/// @returns frequency of the clock parsed
	///
	size_t ParseFrequency(const std::string &clock) const noexcept;



	//-------------------------------------------------------------------------
	//						methods to get result
	//-------------------------------------------------------------------------


	/// @brief get the size of the front port output map
	/// @returns size of the front output map
	///
	inline size_t FrontOutputSize() const noexcept {
		return front_outputs_.size();
	}


	/// @brief get the front output port and source by index
	/// @param[in] index index of the output
	/// @returns an OutputInfo struct includes information of output port index
	///		and output source, otherwise the values in struct are all -1
	///
	inline OutputInfo FrontOutput(size_t index) const noexcept {
		if (index >= front_outputs_.size()) {
			return OutputInfo{size_t(-1), size_t(-1)};
		}
		return front_outputs_[index];
	}


	/// @brief check whether the front port is input port by index
	/// @param[in] index index of port to check
	/// @returns true if is input port, false otherwise
	///
	inline bool IsFrontInput(size_t index) const noexcept {
		if (index >= kFrontIoNum) {
			return false;
		}
		return front_in_use_.test(index);
	}


	/// @brief check whether the front port is output port by index
	/// @param[in] index index of port to check
	/// @returns true if is output port, false otherwise
	///
	inline bool IsFrontOutput(size_t index) const noexcept {
		if (index >= kFrontIoNum) {
			return false;
		}
		return front_out_use_.test(index);
	}


	/// @brief check whether the front port is lemo by index
	/// @param[in] index index of port to check
	/// @returns true if is lemo port, false otherwise
	///
	inline bool IsFrontLemo(size_t index) const noexcept {
		if (index >= kFrontIoNum) {
			return false;
		}
		return front_use_lemo_.test(index);
	}


	/// @brief check whether the front port is logic output only
	/// @param[in] index index of port the check
	/// @returns true if is logic output, false otherwise
	///
	inline bool IsFrontLogicOutput(size_t index) const noexcept {
		if (index >= kFrontIoNum) {
			return false;
		}
		return front_logic_output_.test(index);
	}


	/// @brief get the number of the or gates
	/// @returns size of or gates
	///
	inline size_t OrGateSize() const noexcept {
		return or_gate_size_;
	}


	/// @brief get or-gate bits flags by index
	/// @param[in] index index of or-gate
	/// @returns or-gate flag in array, null if out of range
	///
	inline uint64_t* OrGate(size_t index) const noexcept {
		if (index >= or_gate_size_) return nullptr;
		return (uint64_t*)or_gates_[index];
	}


	/// @brief get the number of the and gates
	/// @returns size of and gates
	///
	inline size_t AndGateSize() const noexcept {
		return and_gate_size_;
	}


	/// @brief get and-gate bits flags by index
	/// @param[in] index index of and-gate
	/// @returns and-gate flag in array, null if out of range
	///
	inline uint64_t* AndGate(size_t index) const noexcept {
		if (index >= and_gate_size_) return nullptr;
		return (uint64_t*)and_gates_[index];
	}


	/// @brief get number of divider-or-gates
	/// @returns size of divider-or-gates
	///
	inline size_t DividerOrGateSize() const noexcept {
		return divider_or_gate_size_;
	}


	/// @brief get divider-or-gates bits flags by index
	/// @param[in] index index of divider-or-gates
	/// @returns divider-or-gate flag in array, null if out of range
	///
	inline uint64_t* DividerOrGate(size_t index) const noexcept {
		if (index >= divider_or_gate_size_) return nullptr;
		return (uint64_t*)divider_or_gates_[index];
	}


	/// @brief get number of divider-and-gates
	/// @returns size of divider-and-gates
	///
	inline size_t DividerAndGateSize() const noexcept {
		return divider_and_gate_size_;
	}


	/// @brief get divider-and-gate bits flags by index
	/// @param[in] index index of divider-and-gates
	/// @returns divider-and-gate flag in array, null if out of range
	///
	inline uint64_t* DividerAndGate(size_t index) const noexcept {
		if (index >= divider_and_gate_size_) return nullptr;
		return (uint64_t*)divider_and_gates_[index];
	}


	/// @brief whether to enable back plane output
	/// @returns true to enable, false otherwise
	///
	inline bool BackEnable() const noexcept {
		return back_output_ != size_t(-1);
	}


	/// @brief get the back plane output source
	/// @returns source index of back output
	///
	inline size_t BackSource() const noexcept {
		return back_output_;
	}

	/// @brief whether to sent clock signal to backplane as external clock
	/// @returns true if enable, false otherwise
	///
	inline bool ExternalClockEnable() const noexcept {
		return extern_clock_ != size_t(-1);
	}


	/// @brief get external clock source
	/// @returns source index of external clock output
	///
	inline size_t ExternalClock() const noexcept {
		return extern_clock_;
	}


	/// @brief get clock size
	/// @returns clock size
	///
	inline size_t ClockSize() const noexcept {
		return clocks_.size();
	}


	/// @brief get clock frequency by index
	/// @param[in] index index of the clock
	/// @returns clock frequency
	///
	inline size_t ClockFrequency(size_t index) const noexcept {
		return clocks_[index];
	}


	/// @brief get the clock which frequency is 1Hz
	/// @returns the index clock which frequency is 1Hz, -1 on failure
	///
	inline size_t SecondClock() const noexcept {
		for (size_t i = 0; i < clocks_.size(); ++i) {
			if (clocks_[i] == 1) {
				return kClocksOffset + i;
			}
		}
		return size_t(-1);
	}


	/// @brief get scaler size
	/// @returns scalers size
	///
	inline size_t ScalerSize() const noexcept {
		return scalers_.size();
	}


	/// @brief get the scaler port and source by index
	/// @param[in] index index of the output
	/// @returns an OutputInfo struct includes information of output port index
	///		and output source, otherwise the values in struct are all -1
	///
	inline OutputInfo Scaler(size_t index) const noexcept {
		if (index >= scalers_.size()) {
			return OutputInfo{size_t(-1), size_t(-1)};
		}
		return scalers_[index];
	}


	/// @brief get the divider size
	/// @returns divider size
	///
	inline size_t DividerSize() const noexcept {
		return dividers_.size();
	}


	/// @brief get the divider information by index
	/// @param[in] index index of divider to get
	/// @returns a struct DividerInfo includes divider port, source and divisor
	///		, or all -1 in struct on failure
	///
	inline DividerInfo Divider(size_t index) const noexcept {
		if (index >= dividers_.size()) {
			return DividerInfo{size_t(-1), size_t(-1)};
		}
		return dividers_[index];
	}

private:
	std::vector<OutputInfo> front_outputs_;
	std::bitset<kFrontIoNum> front_out_use_;
	std::bitset<kFrontIoNum> front_in_use_;
	std::bitset<kFrontIoNum> front_use_lemo_;
	std::bitset<kFrontIoNum> front_logic_output_;

	size_t back_output_;
	size_t extern_clock_;

	// gate size
	size_t or_gate_size_;
	size_t and_gate_size_;
	size_t divider_or_gate_size_;
	size_t divider_and_gate_size_;

	// gate
	uint64_t or_gates_[kMaxOrGates][kOrGateWidth];
	uint64_t and_gates_[kMaxAndGates][kAndGateWidth];
	uint64_t divider_or_gates_[kMaxDividerOrGates][kDividerOrGateWidth];
	uint64_t divider_and_gates_[kMaxDividerAndGates][kDividerAndGateWidth];

	// clocks
	std::vector<size_t> clocks_;

	// scalers
	std::vector<OutputInfo> scalers_;
	std::bitset<kMaxScalers> scaler_use_;

	std::vector<DividerInfo> dividers_;
};

}				// namespace ecl

#endif			// __CONFIG_PARSER_H__