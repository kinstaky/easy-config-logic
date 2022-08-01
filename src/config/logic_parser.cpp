#include "config/logic_parser.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace ecc {

LogicParser::LogicParser() {
	Clear();
}


void LogicParser::Clear() noexcept {
	front_outputs_.clear();
	front_out_use_ = 0;
	front_in_use_ = 0;
	front_use_lemo_ = 0;
	front_logic_output_ = 0;
	back_output_ = size_t(-1);
	or_gates_.clear();
	and_gates_.clear();
	clocks_.clear();
	clocks_.push_back(1);
	scalers_.clear();
	scaler_use_ = 0;
	dividers_.clear();
	divider_use_ = 0;
	divider_gates_.clear();
}


int LogicParser::Read(const std::string &path) noexcept {
	std::ifstream fin;
	fin.open(path, std::ios_base::in);
	if (!fin.good()) {
		std::cerr << "Error open file " << path << std::endl;
		return -1;
	}
	while (fin.good()) {
		// readline
		std::string line;
		std::getline(fin, line);
		if (line.empty()) {
			continue;
		}
		if (Parse(line) != 0) {
			std::cerr << "Error: Parse failure " << line << std::endl;
			return -1;
		}
	}
	fin.close();
	return 0;
}


int LogicParser::Parse(const std::string &expr) noexcept {
	std::string left_side_str;
	std::vector<TokenPtr> tokens;
	size_t divisor;
	if (ExtendLeftLex(expr, left_side_str, tokens, divisor) != 0) {
		// std::cerr << "Error: ExtendLeftLex failed" << std::endl;
		return -1;
	}

	int generate_index = ExtendDividerParse(left_side_str, tokens, divisor);
	if (generate_index < 0) {
		// std::cerr << "Error: ExtendDividerParse failed" << std::endl;
		return -1;
	}

	size_t left_index = IdentifierIndex(left_side_str);
	if (IsFrontIo(left_side_str)) {
		front_outputs_.push_back(OutputInfo{left_index, size_t(generate_index)});
		front_out_use_.set(left_index);
		if (!IsClock(tokens[0]->Value())) {
			front_logic_output_.set(left_index);
		}
		if (IsLemoIo(left_side_str)) {
			front_use_lemo_.set(left_index);
		}
	} else if (IsBack(left_side_str)) {
		back_output_ = generate_index;
	} else if (IsScaler(left_side_str)) {
		scalers_.push_back(OutputInfo{left_index-kScalersOffset, size_t(generate_index)});
		scaler_use_.set(left_index-kScalersOffset);
	} else if (IsDivider(left_side_str)) {
		dividers_.push_back(DividerInfo{left_index-kDividersOffset, size_t(generate_index), divisor});
		divider_use_.set(left_index-kDividersOffset);
	}

	return 0;
}


int LogicParser::ExtendLeftLex(const std::string &expr, std::string &left, std::vector<TokenPtr> &tokens, size_t &divisor) const noexcept {
	std::string right;
	if (LeftSideLex(expr, left, right) != 0) {
		return -1;
	}

	if (ExtendDividerLex(right, tokens, divisor) != 0) {
		return -1;
	}

	return 0;
}


int LogicParser::ExtendDividerLex(const std::string &expr, std::vector<TokenPtr> &tokens, size_t &divisor) const noexcept {
	tokens.clear();
	divisor = 0;

	size_t n = expr.rfind('/');
	if (n == std::string::npos) {
		// no divider, so the whole expression can pass to lexer
		n = expr.length();
	} else {
		// contains divider, so extract the divisor first
		// check divisor
		std::string divisor_str = expr.substr(n+1, expr.length()-n-1);
		for (char c : divisor_str) {
			if (c == ' ') {
				continue;
			}
			if (c < '0' || c > '9') {
				return -1;
			}
		}
		// convert string to number
		std::stringstream ss;
		ss << divisor_str;
		ss >> divisor;
		if (divisor == 0) {
			return -1;
		}
	}

	Lexer lexer;
	if (lexer.Analyse(expr.substr(0, n), tokens) != 0) {
		return -1;
	}
	if (tokens.empty()) {
		return -1;
	}
	return 0;
}


int LogicParser::LeftSideLex(const std::string &expr, std::string &left, std::string &right) const noexcept {
	left = "";
	right = "";

	for (size_t i = 0; i < expr.length(); ++i) {
		const char &c = expr[i];

		// ignore the blank character
		if (c == ' ') {
			continue;
		}

		if (c == '=') {
			if (left.empty()) {
				// std::cerr << "Error: Expected left hand side variable before '='" << std::endl;
				return -1;
			} else {
				right = expr.substr(i+1, expr.length()-i-1);
				return 0;
			}
		} else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
			left += c;
		} else if (c >= '0' && c <= '9') {
			left += c;
		} else {
			if (left.empty()) {
				// std::cerr << "Error: Expected expression starts with left hand side variable!" << std::endl;
			} else {
				// std::cerr << "Error: Invalid symbol in left hand side variable: " << c << std::endl;
			}
			return -1;
		}
	}
	return 0;
}


int LogicParser::ExtendDividerParse(const std::string &left, const std::vector<TokenPtr> &right, size_t divisor) noexcept {
	// check identifiers form
	if (!CheckIdentifiers(left, right)) {
		// std::cerr << "Error: Identifiers form!" << std::endl;
		return -1;
	}
	// check port input and output conflict
	if (!CheckIoConflict(left, right)) {
		// std::cerr << "Error: Input output conflict!" << std::endl;
		return -1;
	}

	if (IsDivider(left) && !divisor) {
		// std::cerr << "Error: Expected \" / divisor\" in divider expression" << std::endl;
		return -1;
	}

	if (right.size() == 1 && !IsClock(right[0]->Value())) {
		size_t id_index = IdentifierIndex(right[0]->Value());
		if (IsFrontIo(right[0]->Value())) {
			front_in_use_.set(id_index);
			if (IsLemoIo(right[0]->Value())) {
				front_use_lemo_.set(id_index);
			}
		}
		return id_index;
	}


	// parse the right side tokens and generate the syntax tree
	LogicalGrammar grammar;
	SLRSyntaxParser parser(&grammar);

	if (IsDivider(right[0]->Value())) {
		// contains divider
		std::vector<TokenPtr> extracted_tokens(right.begin()+2, right.end());
		if (parser.Parse(extracted_tokens) != 0) {
			// std::cerr << "Error: Syntax parse failed" << std::endl;
			return -1;
		}
	} else {
		if (parser.Parse(right) != 0) {
			// std::cerr << "Error: Syntax parse failed" << std::endl;
			return -1;
		}
	}

	// standardize the syntax tree
	StandardLogicTree tree(parser.Root());

	// generate gates
	int generate_index = GenerateGates(tree.Root(), tree.IdList());
	if (generate_index < 0) {
		// std::cerr << "Error: Generate gates!" << std::endl;
		return -1;
	}

	if (IsDivider(right[0]->Value())) {
		std::vector<TokenPtr> divider_tokens(right.begin(), right.begin()+2);
		generate_index = GenerateDividerGate(divider_tokens, size_t(generate_index));
		if (generate_index < 0) {
			// std::cerr << "Error: Generate divider gate" << std::endl;
			return -1;
		}
	}

	return generate_index;
}


bool LogicParser::CheckIdentifiers(const std::string &left, const std::vector<TokenPtr> &right) const noexcept {
	// check left side identifier
	if (!IsFrontIo(left) && !IsBack(left) && !IsScaler(left) && !IsDivider(left)) {
		return false;
	}
	// check right side identifier
	if (right.size() == 1) {
		if (right[0]->Type() != kSymbolType_Identifier) {
			return false;
		}
		if (IsClock(right[0]->Value())) {
			if (!IsFrontIo(left)) {
				// if right is clock, left must be front io port
				return false;
			}
		} else if (IsDivider(right[0]->Value())) {
			if (IsDivider(left)) {
				// if right is divider, left can be front io port, back or
				// scaler, but can't be divider
				return false;
			}
		} else if (!IsFrontIo(right[0]->Value())) {
			return false;
		}
	} else {
		auto id_start = right.begin();
		if (IsDivider(right[0]->Value())) {
			if (right[1]->Value() != "|" && right[1]->Value() != "&") {
				// std::cerr << "Error: Expected operator '|' or '&' after divider identifier " << right[1]->Value() << std::endl;
				return false;
			}
			std::advance(id_start, 2);
		}
		for (auto it = id_start; it != right.end(); ++it) {
			auto &id = *it;
			if (id->Type() == kSymbolType_Operator) {
				if (id->Value() != "|" && id->Value() != "&"
					&& id->Value() != "(" && id->Value() != ")"
				){
					// std::cerr << "Error: Undefined operator " << id->Value() << std::endl;
					return false;
				}
			} else if (id->Type() == kSymbolType_Identifier) {
				if (!IsFrontIo(id->Value())) {
					// std::cerr << "Error: Expected identifier in front io port form " << id->Value() << std::endl;
					return false;
				}
			} else {
				// std::cerr << "Error: Invalid identifier type " << id->Type() << std::endl;
				return false;
			}
		}
	}
	return true;
}



bool LogicParser::CheckIoConflict(const std::string &left, const std::vector<TokenPtr> &right) const noexcept {
	// check output conflict, i.e. an output port with two sources
	if (IsBack(left)) {
		// check back output conflict
		if (back_output_ != size_t(-1)) {
			// two source of back
			// std::cerr << "Error: Multiple source of back plane port" << std::endl;
			return false;
		}
	} else if (IsFrontIo(left)) {
		// check front output port conflict
		if (front_out_use_.test(IdentifierIndex(left))) {
			// front output conflict
			// std::cerr << "Error: Multiple source of fornt port " << left << std::endl;
			return false;
		}
	} else if (IsScaler(left)) {
		// check scaler source conflict
		if (scaler_use_.test(IdentifierIndex(left)-kScalersOffset)) {
			// scaler source conflict
			// std::cerr << "Error: Multiple source of scaler " << left << std::endl;
			return false;
		}
	} else if (IsDivider(left)) {
		// check divider source conflict
		if (divider_use_.test(IdentifierIndex(left)-kDividersOffset)) {
			// divider source conflict
			// std::cerr << "Error: Multiple source of divider " << left << std::endl;
			return false;
		}
	}

	// check whether the using divider has been defined before
	if (IsDivider(right[0]->Value()) && !divider_use_.test(IdentifierIndex(right[0]->Value())-kDividersOffset)) {
		// std::cerr << "Error: Use of undefined divider " << right[0]->Value() << std::endl;
		return false;
	}


	// check input output conflict, i.e. a port is both an input and output port
	// check itself consistent, expect no port both in left and right
	if (IsFrontIo(right[0]->Value())) {
		// not the clock
		for (size_t i = 0; i < right.size(); ++i) {
			if (right[i]->Type() != kSymbolType_Identifier) {
				continue;
			}
			if (right[i]->Value() == left) {
				// std::cerr << "Error: Input and output in the same port " << left << std::endl;
				return false;
			}
		}
	}

	// check this output with previous inputs
	if (IsFrontIo(left) && front_in_use_.test(IdentifierIndex(left))) {
		// std::cerr << "Error: Output port defined as input port before " << left << std::endl;
		return false;
	}
	// check this inputs with previous outputs
	if (!IsClock(right[0]->Value())) {
		// not the clock
		if (IsScaler(left) && right.size() == 1) {
			if (right[0]->Type() != kSymbolType_Identifier) {
				// std::cerr << "Error: The only token is not identifier " << right[0]->Value() << std::endl;
				return false;
			}
		} else {
			for (size_t i = 0; i < right.size(); ++i) {
				if (right[i]->Type() != kSymbolType_Identifier) {
					// ignore operators
					continue;
				}
				if (!IsFrontIo(right[i]->Value())) {
					// ignore dividers
					continue;
				}
				if (front_out_use_.test(IdentifierIndex(right[i]->Value()))) {
					// std::cerr << "Error: Input port defined as output port before " << right[i]->Value() << std::endl;
					return false;
				}
			}
		}
	}

	// check lemo input conflict, i.e. an input port defined as both lemo and not
	for (size_t i = 0; i < right.size(); ++i) {
		if (right[i]->Type() != kSymbolType_Identifier) {
			continue;
		}
		if (IsFrontIo(right[i]->Value())) {
			if (!front_in_use_.test(IdentifierIndex(right[i]->Value()))) {
				continue;
			}
			if (front_use_lemo_.test(IdentifierIndex(right[i]->Value()))) {
				if (!IsLemoIo(right[i]->Value())) {
					// std::cerr << "Error: Input port was defined as lemo input before " << right[i]->Value() << std::endl;
					return false;
				}
			} else {
				if (IsLemoIo(right[i]->Value())) {
					// std::cerr << "Error: Input port wasn't defined as lemo input before " << right[i]->Value() << std::endl;
					return false;
				}
			}
		}
	}

	return true;
}



bool LogicParser::IsFrontIo(const std::string &name) const noexcept {
	if (name.length() < 2 || name.length() > 3) {
		return false;
	}
	if (name[0] < 'A' || name[0] > 'C') {
		// first character is not A, B or C
		return false;
	}
	for (size_t i = 1; i < name.length(); ++i) {
		if (name[i] < '0' || name[i] > '9') {
			return false;
		}
	}
	std::stringstream ss;
	ss << name.substr(1, name.length()-1);
	size_t port_index;
	ss >> port_index;
	if (port_index >= 16*2) {
		return false;
	}
	return true;
}


bool LogicParser::IsLemoIo(const std::string &name) const noexcept {
	if (name.length() < 2 || name.length() > 3) {
		return false;
	}
	if (name[0] < 'A' || name[0] > 'C') {
		// first character is not A, B or C
		return false;
	}
	for (size_t i = 1; i < name.length(); ++i) {
		if (name[i] < '0' || name[i] > '9') {
			return false;
		}
	}
	std::stringstream ss;
	ss << name.substr(1, name.length()-1);
	size_t port_index;
	ss >> port_index;
	if (port_index < 16 || port_index >= 16*2) {
		return false;
	}
	return true;
}


bool LogicParser::IsClock(const std::string &name) const noexcept {
	if (name.length() < 9) {
		return false;
	}
	if (name.substr(0, 6) != "clock_") {
		return false;
	}
	if (name.substr(name.length()-2, 2) != "Hz") {
		return false;
	}

	bool gain = false;
	if (name[name.length()-3] == 'k' || name[name.length()-3] == 'M') {
		gain = true;
	}

	size_t suffix_size = gain ? 3 : 2;
	for (size_t i = 6; i < name.length()-suffix_size; ++i) {
		if (name[i] < '0' || name[i] > '9') {
			return false;
		}
	}
	return true;
}


bool LogicParser::IsScaler(const std::string &name) const noexcept {
	if (name[0] != 'S') {
		return false;
	}
	for (size_t i = 1; i < name.length(); ++i) {
		if (name[i] < '0' || name[i] > '9') {
			return false;
		}
	}
	std::stringstream ss;
	ss << name.substr(1, name.length()-1);
	size_t scaler_index;
	ss >> scaler_index;
	if (scaler_index >= kMaxScalers) {
		return false;
	}
	return true;
}


bool LogicParser::IsDivider(const std::string &name) const noexcept {
	if (name[0] != 'D') {
		return false;
	}
	for (size_t i = 1; i < name.length(); ++i) {
		if (name[i] < '0' || name[i] > '9') {
			return false;
		}
	}
	std::stringstream ss;
	ss << name.substr(1, name.length()-1);
	size_t index;
	ss >> index;
	if (index >= kMaxDividers) {
		return false;
	}
	return true;
}



size_t LogicParser::IdentifierIndex(const std::string &id) const noexcept {
	if (IsBack(id)) {
		return kBackOffset;
	}

	if (IsClock(id)) {
		size_t frequency = ParseFrequency(id);
		for (size_t i = 0; i < clocks_.size(); ++i) {
			if (frequency == clocks_[i]) {
				return kClocksOffset + i;
			}
		}
		return size_t(-1);
	}

	// is front io port or scaler or divider
	size_t result;
	std::stringstream ss;
	ss << id.substr(1, id.length()-1);
	ss >> result;

	if (IsFrontIo(id)) {
		// lemo form
		result -= IsLemoIo(id) ? 16 : 0;
		// normal form
		result += id[0] == 'B' ? 16 : 0;
		result += id[0] == 'C' ? 32 : 0;
	} else if (IsScaler(id)) {
		result += kScalersOffset;
	} else {
		// is divider
		result += kDividersOffset;
	}

	return result;
}


int LogicParser::GenerateGates(StandardLogicNode *root, const std::vector<Identifier*> &id_list) noexcept {
	// check the operator type first
	if (root->OperatorType() == kOperatorNull) {
		// null means only one identifier, just return io port index
		for (size_t i = 0; i < kMaxIdentifier; ++i) {
			if (root->Leaves().test(i)) {
				// found the identifier
				if (IsFrontIo(id_list[i]->Value())) {
					// is front io port, add it to input used and return
					size_t id_index = IdentifierIndex(id_list[i]->Value());
					front_in_use_.set(id_index);
					if (IsLemoIo(id_list[i]->Value())) {
						front_use_lemo_.set(id_index);
					}
					return id_index;

				} else if (IsClock(id_list[i]->Value())) {
					// is clock, generate a clock
					return GenerateClock(id_list[i]->Value());
				}
			}
		}
	} else if (root->OperatorType() == kOperatorOr) {
		// assert or nodes without branch
		if (root->BranchSize()) {
			return -1;
		}
		return GenerateOrGate(root->Leaves(), id_list);
	} else if (root->OperatorType() == kOperatorAnd) {
		// assert depth less than or equal 2
		if (root->Depth() > 2) {
			return -1;
		}

		// set bits matching FrontIo ports
		std::bitset<kAndBits> and_bits = 0;
		std::bitset<kMaxIdentifier> and_leaves = root->Leaves();
		for (size_t i = 0; i < kMaxIdentifier && and_bits.count() < and_leaves.count(); ++i) {
			if (and_leaves.test(i)) {
				// add this identifier to input used and add to the and-gate
				size_t id_index = IdentifierIndex(id_list[i]->Value());
				front_in_use_.set(id_index);
				and_bits.set(id_index);
				if (IsLemoIo(id_list[i]->Value())) {
					front_use_lemo_.set(id_index);
				}
			}
		}

		// generate or gates from its branches
		for (size_t i = 0; i < root->BranchSize(); ++i) {
			int gate_index = GenerateOrGate(root->Branch(i)->Leaves(), id_list);
			if (gate_index < 0) {
				return -1;
			}
			and_bits.set(gate_index);
		}

		// check existense of and_bits
		for (size_t i = 0; i < and_gates_.size(); ++i) {
			if (and_gates_[i] == and_bits) {
				return kAndGatesOffset + i;
			}
		}
		// not exist
		if (and_gates_.size() < kMaxAndGates) {
			and_gates_.push_back(and_bits);
			return kAndGatesOffset + and_gates_.size() - 1;
		}
	}

	return -1;
}



int LogicParser::GenerateOrGate(std::bitset<kMaxIdentifier> id_flag, std::vector<Identifier*> id_list) noexcept {
	std::bitset<kOrBits> or_bits;
	for (size_t i = 0; i < kMaxIdentifier && or_bits.count() < id_flag.count(); ++i) {
		if (id_flag.test(i)) {
			// set the identifier in front input use and add to or-gate
			size_t id_index = IdentifierIndex(id_list[i]->Value());
			front_in_use_.set(id_index);
			or_bits.set(id_index);
			if (IsLemoIo(id_list[i]->Value())) {
				front_use_lemo_.set(id_index);
			}
		}
	}

	// search existence of or_bits
	for (size_t i = 0; i < or_gates_.size(); ++i) {
		if (or_gates_[i] == or_bits) {
			return kOrGatesOffset + i;
		}
	}
	// not exist
	if (or_gates_.size() < kMaxOrGates) {
		or_gates_.push_back(or_bits);
		return kOrGatesOffset + or_gates_.size()-1;
	}
	return -1;
}



int LogicParser::GenerateClock(const std::string &id) noexcept {
	size_t frequency = ParseFrequency(id);
	// check existence
	for (size_t i = 0; i < clocks_.size(); ++i) {
		if (clocks_[i] == frequency) {
			return kClocksOffset + i;
		}
	}
	// not exists
	if (clocks_.size() < kMaxClocks) {
		clocks_.push_back(frequency);
		return kClocksOffset + clocks_.size() - 1;
	}
	return -1;
}


size_t LogicParser::ParseFrequency(const std::string &clock) const noexcept {
	// parse gain
	size_t gain = 1u;
	if (clock[clock.length()-3] == 'k') {
		gain = 1000u;
	} else if (clock[clock.length()-3] == 'M') {
		gain = 1'000'000u;
	}

	// parse numbers
	size_t suffix_size = gain == 1 ? 2 : 3;
	std::stringstream ss;
	ss << clock.substr(6, clock.length()-suffix_size);
	size_t frequency;
	ss >> frequency;

	// calculate frequency
	frequency *= gain;
	return frequency;
}


int LogicParser::GenerateDividerGate(const std::vector<TokenPtr> &tokens, size_t other_source) noexcept {
	size_t divider_index = IdentifierIndex(tokens[0]->Value()) - kDividersOffset;
	int op_type = 0;
	op_type = tokens[1]->Value() == "|" ? kOperatorOr : op_type;
	op_type = tokens[1]->Value() == "&" ? kOperatorAnd : op_type;
	if (op_type == 0) {
		// std::cerr << "Error: Invalid operator " << tokens[1]->Value() << std::endl;
		return -1;
	}

	// check existence
	for (size_t i = 0; i < divider_gates_.size(); ++i) {
		DividerGateInfo &info = divider_gates_[i];
		if (info.divider == divider_index && info.source == other_source && info.op_type == op_type) {
			return kDividerGatesOffset + i;
		}
	}
	// not exists
	divider_gates_.push_back(DividerGateInfo{divider_index, other_source, op_type});
	return kDividerGatesOffset + divider_gates_.size() - 1;
}


}	 			// namespace ecc