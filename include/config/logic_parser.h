#ifndef __LOGIC_PARSER_H__
#define __LOGIC_PARSER_H__


#include <bitset>
#include <map>
#include <string>
#include <vector>

#include "syntax/parser/token.h"
#include "syntax/parser/lexer.h"
#include "syntax/parser/syntax_parser.h"
#include "syntax/logical_grammar.h"
#include "standardize/standard_logic_tree.h"

namespace ecc {

const size_t kFrontIoNum = 48;

const size_t kOrGatesOffset = kFrontIoNum;
const size_t kOrBits = kFrontIoNum;
const size_t kMaxOrGates = 16;

const size_t kAndGatesOffset = kOrGatesOffset + kMaxOrGates;
const size_t kAndBits = kOrBits + kMaxOrGates;
const size_t kMaxAndGates = 16;

const size_t kClocksOffset = kAndGatesOffset + kMaxAndGates;
const size_t kMaxClocks = 4;

const size_t kScalersOffset = kClocksOffset + kMaxClocks;
const size_t kMaxScalers = 32;

const size_t kBackOffset = kScalersOffset + kMaxScalers;
const size_t kMaxBack = 1;

const size_t kDividersOffset = kBackOffset + kMaxBack;
const size_t kMaxDividers = 4;
const size_t kDividerGatesOffset = kDividersOffset + kMaxDividers;
const size_t kMaxDividerGates = 8;

const size_t kPrimaryClockOffset = kDividerGatesOffset + kMaxDividerGates;


struct OutputInfo {
	size_t port;			// port local index
	size_t source;			// source global index
};
struct DividerInfo {
	size_t port;			// divider local index
	size_t source;			// source global index
	size_t divisor;			// divisor
};
struct DividerGateInfo {
	size_t divider;			// index of divider
	size_t source;			// index of the other source
	int op_type;			// operator type
};


class LogicParser {
public:

	/// @brief constuctor
	///
	LogicParser();


	/// @brief destructor
	///
	~LogicParser() = default;


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


	/// @brief extend the lexer to lex the expression contains assignment
	/// @note This method extends the Lexer to lex the assignment expression.
	/// 	It recognizes the operator '=' and divides the left and right side
	/// 	expressions.
	///
	/// @param[in] expr the whole expression includes assignment
	/// @param[out] left the picked left side expression
	/// @param[out] tokens the right side tokens without divider
	/// @param[out] divisor extracted divisor from divider, 0 for no dividers
	/// @returns 0 on success, -1 on failure
	///
	int ExtendLeftLex(const std::string &expr, std::string &left, std::vector<TokenPtr> &tokens, size_t &divisor) const noexcept;


	/// @brief extend the lexer to lex the expression contians operator '/'
	/// @note The Analyse method in Lexer can't recognize both the operator '/'
	/// 	and the following number in divider expression. So this function
	///		pick them from the expression to adapt to the Lexer.
	///
	/// @param[in] expr extend logic expression(inclueds divider)
	/// @param[out] tokens recognize tokens without divider
	/// @param[out] divisor extracted divisor, 0 for no dividers
	/// @returns 0 on success, -1 on failure
	///
	int ExtendDividerLex(const std::string &expr, std::vector<TokenPtr> &tokens, size_t &divisor) const noexcept;



	/// @brief extend the lexer to lex the left side identifier and operator '='
	///
	/// @param[in] expr expression to lex
	/// @param[in] left left side lex result
	/// @param[in] right right side lex result
	/// @returns 0 on success, -1 on failure
	///
	int LeftSideLex(const std::string &expr, std::string &left, std::string &right) const noexcept;


	/// @brief lex the right side of divider definition expression
	///
	/// @param[inout] right input the whole right side string and output the
	///		expression without divider part
	/// @param[out] divisor divisor from the right side string after '/'
	/// @returns 0 on success, -1 on failure
	///
	int DividerLex(std::string &right, size_t &divisor) const noexcept;


	/// @brief parse the expressions includes divider
	///
	/// @param[in] left left side string
	/// @param[in] right right side tokens without divider
	/// @param[in] divisor divisor value calculated in lex process
	/// @returns generated (identifier, gate, clock, scaler, divider) index,
	/// 	-1 on failure
	///
	int ExtendDividerParse(const std::string &left, const std::vector<TokenPtr> &right, size_t divisor) noexcept;


	/// @brief check the form of identifiers
	///
	/// @param[in] left left side identifier in string
	/// @param[in] right right side identifiers in vector
	/// @returns true if identifier in appropirate form, false otherwise
	///
	bool CheckIdentifiers(const std::string &left, const std::vector<TokenPtr> &right) const noexcept;


	/// @brief check whether there is io conflict in
	///
	/// @param[in] left left side identifier name
	/// @param[in] right right side identifier list
	/// @returns true if no conflict up to now, false otherwise
	///
	bool CheckIoConflict(const std::string &left, const std::vector<TokenPtr> &right) const noexcept;



	/// @brief check the identifier is in FrontIo form
	/// @note FrontIO form means that the identifier matches one of the
	/// 	input/output port on MZTIO. It should be A0-A31, B0-B31 or C0-C31.
	///		Apparently, lemo io form is included in FrontIo form.
	/// 	Be aware of the first letter should be uppercase.
	///
	/// @param[in] name the identifier name to check
	/// @returns true if in FrontIO form, false otherwise
	///
	bool IsFrontIo(const std::string &name) const noexcept;


	/// @brief check the identifier is in lemo io form
	/// @note Lemo io form means that the identifier matches one of the
	/// 	input/ouput port on daughter card of MZTIO. It should be
	///		A16-A31, B16-B31, or C116-C31. It's similar to the FrontIo form.
	///
	/// @param[in] name the identifier name to check
	/// @returns true if in lemo io form, false otherwise
	///
	bool IsLemoIo(const std::string &name) const noexcept;


	/// @brief check whether the identifier represents the backplane port
	///
	/// @param[in] name the identifier name to check
	/// @returns true if is backplane port, false otherwise
	///
	inline bool IsBack(const std::string &name) const noexcept {
		return name == "Back";
	}

	/// @brief check whether the identifier represents a clock
	///
	/// @param[in] name the identifier name to check
	/// @returns true if is clock, false otherwise
	///
	bool IsClock(const std::string &name) const noexcept;


	/// @brief check whether the identifier represents a scaler
	///
	/// @param[in] name the identifier name to check
	/// @returns true if is a scaler, false otherwise
	///
	bool IsScaler(const std::string &name) const noexcept;



	/// @brief check whether the identifier represents a divider
	///
	/// @param[in] name the identifier name to check
	/// @returns true if is a divider, false otherwise
	///
	bool IsDivider(const std::string &name) const noexcept;


	/// @brief get the identifier index base on its name
	/// @note The identifier index affect the bitset of or-gates and and-gates.
	/// 	To be convenient, the index 0-47 is FrontIO identifiers. Port
	/// 	A0-A15 matches index 0-15, port B0-B15 matches index 16-31, port
	///		C0-C15 matches index 32-47.
	///
	/// @param[in] id the name of the identifier
	/// @returns index of the identifier
	///
	size_t IdentifierIndex(const std::string &id) const noexcept;


	/// @brief generate or-gates and and-gates from standard logic tree
	///
	/// @param[in] root root of standard logic tree
	/// @param[in] id_list identifier list from the standard logic tree
	/// @returns front io port or gate index if success, -1 otherwise
	///
	int GenerateGates(StandardLogicNode *root, const std::vector<Identifier*> &id_list) noexcept;


	/// @brief  generate an or gate
	///
	/// @param[in] id_flag leaves of the node
	/// @param[in] id_list identifier list from the standard logic tree
	/// @returns or gate index, or -1 on failure
	///
	int GenerateOrGate(std::bitset<kMaxIdentifier> id_flag, const std::vector<Identifier*> id_list) noexcept;


	/// @brief generate a clock according to the identifier name
	///
	/// @param[in] id name of identifier
	/// @returns generated clock index, or -1 on failure
	///
	int GenerateClock(const std::string &id) noexcept;


	//// @brief generate or-gate or and-gate with divider output
	///
	/// @param[in] tokens tokens of the divider and operator
	/// @param[in] other_source index of the the other source to operate with divider
	/// @returns genrated index, or -1 on failure
	///
	int GenerateDividerGate(const std::vector<TokenPtr> &tokens, size_t other_source) noexcept;


	/// @brief parse the clock name and get the frequency
	///
	/// @param[in] clock the name of the clock identifier
	/// @returns frequency of the clock parsed
	///
	size_t ParseFrequency(const std::string &clock) const noexcept;



	//-------------------------------------------------------------------------
	//						methods to get result
	//-------------------------------------------------------------------------


	/// @brief get the size of the front port output map
	///
	/// @returns size of the front output map
	///
	inline size_t FrontOutputSize() const noexcept {
		return front_outputs_.size();
	}


	/// @brief get the front output port and source by index
	///
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
	///
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
	///
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
	///
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
	///
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
	///
	/// @returns size of or gates
	///
	inline size_t OrGateSize() const noexcept {
		return or_gates_.size();
	}


	/// @brief get or-gate bits flags by index
	///
	/// @param[in] index index of or-gate
	/// @returns or-gate bits flag, 0 otherwise
	///
	inline std::bitset<kOrBits> OrGate(size_t index) const noexcept {
		if (index >= or_gates_.size()) {
			return 0;
		}
		return or_gates_[index];
	}



	/// @brief get the number of the and gates
	///
	/// @returns size of and gates
	///
	inline size_t AndGateSize() const noexcept {
		return and_gates_.size();
	}



	/// @brief get and-gate bits flags by index
	///
	/// @param[in] index index of and-gate
	/// @returns and-gate bits flag, 0 otherwise
	///
	inline std::bitset<kAndBits> AndGate(size_t index) const noexcept {
		if (index >= and_gates_.size()) {
			return 0;
		}
		return and_gates_[index];
	}


	/// @brief whether to enable back plane output
	///
	/// @returns true to enable, false otherwise
	///
	inline bool BackEnable() const noexcept {
		return back_output_ != size_t(-1);
	}


	/// @brief get the back plane output source
	///
	/// @returns source index of back output
	///
	inline size_t BackSource() const noexcept {
		return back_output_;
	}


	/// @brief get clock size
	///
	/// @returns clock size
	///
	inline size_t ClockSize() const noexcept {
		return clocks_.size();
	}


	/// @brief get clock frequency by index
	///
	/// @param[in] index index of the clock
	/// @returns clock frequency
	///
	inline size_t ClockFrequency(size_t index) const noexcept {
		return clocks_[index];
	}

	/// @brief get the clock which frequency is 1Hz
	///
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
	///
	/// @returns scalers size
	///
	inline size_t ScalerSize() const noexcept {
		return scalers_.size();
	}


	/// @brief get the scaler port and source by index
	///
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
	///
	/// @returns divider size
	///
	inline size_t DividerSize() const noexcept {
		return dividers_.size();
	}


	/// @brief get the divider information by index
	///
	/// @param[in] index index of divider to get
	/// @returns a struct DividerInfo includes divider port, source and divisor
	///		, or all -1 in struct on failure
	///
	inline DividerInfo Divider(size_t index) const noexcept {
		if (index >= dividers_.size()) {
			return DividerInfo{size_t(-1), size_t(-1), size_t(-1)};
		}
		return dividers_[index];
	}


	/// @brief get the divider gate size
	///
	/// @returns size of divider gates
	///
	inline size_t DividerGateSize() const noexcept {
		return divider_gates_.size();
	}


	/// @brief get the divider gate information by index
	///
	/// @param[in] index index of divider gate to get
	/// @returns a struct DividerGateInfo includes divider source, the other
	/// 	source and operator type of divider gate
	///
	inline DividerGateInfo DividerGate(size_t index) const noexcept {
		if (index >= divider_gates_.size()) {
			return DividerGateInfo{size_t(-1), size_t(-1), -1};
		}
		return divider_gates_[index];
	}




private:
	std::vector<OutputInfo> front_outputs_;
	std::bitset<kFrontIoNum> front_out_use_;
	std::bitset<kFrontIoNum> front_in_use_;
	std::bitset<kFrontIoNum> front_use_lemo_;
	std::bitset<kFrontIoNum> front_logic_output_;

	size_t back_output_;

	std::vector<std::bitset<kOrBits>> or_gates_;

	std::vector<std::bitset<kAndBits>> and_gates_;

	std::vector<size_t> clocks_;

	std::vector<OutputInfo> scalers_;
	std::bitset<kMaxScalers> scaler_use_;

	std::vector<DividerInfo> dividers_;
	std::bitset<kMaxDividers> divider_use_;
	std::vector<DividerGateInfo> divider_gates_;
};

}				// namespace ecc

#endif			// __LOGIC_PARSER_H__