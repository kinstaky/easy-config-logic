#include "config/config_parser.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "syntax/parser/lexer.h"
#include "syntax/parser/syntax_parser.h"
#include "syntax/logic_downscale_grammar.h"

namespace ecl {

ConfigParser::ConfigParser() {
	Clear();
}


void ConfigParser::Clear() noexcept {
	front_outputs_.clear();
	front_out_use_ = 0;
	front_in_use_ = 0;
	front_use_lemo_ = 0;
	front_logic_output_ = 0;
	back_output_ = size_t(-1);
	extern_clock_ = size_t(-1);
	or_gates_.clear();
	and_gates_.clear();
	clocks_.clear();
	clocks_.push_back(1);
	scalers_.clear();
	scaler_use_ = 0;
	dividers_.clear();
	divider_use_ = 0;
}


int ConfigParser::Read(const std::string &path) noexcept {
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


int ConfigParser::Parse(const std::string &expr) noexcept {
	// lexer analysis
	Lexer lexer;
	// tokens
	std::vector<TokenPtr> tokens;
	// get tokens
	int lex_result = lexer.Analyse(expr, tokens);
	if (lex_result == -1) {
		std::cerr << "Error: Invalid character in expression: "
			<< expr << "\n";
		return -1;
	} else if (lex_result == -2) {
		std::cerr << "Error: Variable starts with digits: "
			<< expr << "\n";
		return -1;
	} else if (lex_result == -3) {
		std::cerr << "Error: Variable starts with underscore '_': "
			<< expr << "\n";
		return -1;
	}

	// check identifiers form
	if (!CheckIdentifiers(tokens)) {
		std::cerr << "Error: Identifiers form!" << std::endl;
		return -1;
	}
	// check port input and output conflict
	if (!CheckIoConflict(tokens)) {
		std::cerr << "Error: Input output conflict!" << std::endl;
		return -1;
	}

	// grammar
	LogicDownscaleGrammar grammar;
	// parser
	SLRSyntaxParser<int> parser(&grammar);
	// parse tokens
	if (parser.Parse(tokens)) {
		std::cerr << "Error: Parse failed: "
			<< expr << "\n";
		return -2;
	}
	// check nested downscale
	if (parser.Eval() >= 2) {
		std::cerr << "Error: Unable to parse expression with nested downscale expression."
			<< expr << "\n";
		return -2;
	}

	// standardize
	StandardLogicDownscaleTree tree((Production<int>*)(parser.Root()->Child(2)));

	int generate_index = -1;
	if (tree.Root()->OperatorType() == kOperatorNull) {
		GatesInfo<kAndBits> gate_info{and_gates_, kAndGatesOffset, kMaxAndGates};
		generate_index = GenerateGate(&tree, tree.Root(), 0, gate_info);
	} else if (tree.Root()->OperatorType() == kOperatorOr) {
		GatesInfo<kOrBits> gate_info{or_gates_, kOrGatesOffset, kMaxOrGates};
		int layer = parser.Eval() == 0 ? 1 : 3;
		generate_index = GenerateGate(&tree, tree.Root(), layer, gate_info);
	} else if (tree.Root()->OperatorType() == kOperatorAnd) {
		GatesInfo<kAndBits> gate_info{and_gates_, kAndGatesOffset, kMaxAndGates};
		int layer = parser.Eval() == 0 ? 2 : 4;
		generate_index = GenerateGate(&tree, tree.Root(), layer, gate_info);
	}
	if (generate_index < 0) {
		std::cerr << "Error: Generate gates failed.\n";
		return -1;
	}

	// left side token name
	std::string left_name = tokens[0]->Name();
	size_t left_index = IdentifierIndex(left_name);
	if (IsFrontIo(left_name)) {
		front_outputs_.push_back(OutputInfo{left_index, size_t(generate_index)});
		front_out_use_.set(left_index);
		if (!IsClock(tokens[2]->Name())) {
			front_logic_output_.set(left_index);
		}
		if (IsLemoIo(left_name)) {
			front_use_lemo_.set(left_index);
		}
	} else if (IsBack(left_name)) {
		back_output_ = generate_index;
	} else if (IsScaler(left_name)) {
		scalers_.push_back(OutputInfo{left_index-kScalersOffset, size_t(generate_index)});
		scaler_use_.set(left_index-kScalersOffset);
	} else if (IsExternalClock(left_name)) {
		extern_clock_ = generate_index;
	}

	return 0;
}


// int ConfigParser::ExtendLeftLex(const std::string &expr, std::string &left, std::vector<TokenPtr> &tokens, size_t &divisor) const noexcept {
// 	std::string right;
// 	if (LeftSideLex(expr, left, right) != 0) {
// 		return -1;
// 	}

// 	if (ExtendDividerLex(right, tokens, divisor) != 0) {
// 		return -1;
// 	}

// 	return 0;
// }


// int ConfigParser::ExtendDividerLex(const std::string &expr, std::vector<TokenPtr> &tokens, size_t &divisor) const noexcept {
// 	tokens.clear();
// 	divisor = 0;

// 	size_t n = expr.rfind('/');
// 	if (n == std::string::npos) {
// 		// no divider, so the whole expression can pass to lexer
// 		n = expr.length();
// 	} else {
// 		// contains divider, so extract the divisor first
// 		// check divisor
// 		std::string divisor_str = expr.substr(n+1, expr.length()-n-1);
// 		for (char c : divisor_str) {
// 			if (c == ' ') {
// 				continue;
// 			}
// 			if (c < '0' || c > '9') {
// 				return -1;
// 			}
// 		}
// 		// convert string to number
// 		std::stringstream ss;
// 		ss << divisor_str;
// 		ss >> divisor;
// 		if (divisor == 0) {
// 			return -1;
// 		}
// 	}

// 	Lexer lexer;
// 	if (lexer.Analyse(expr.substr(0, n), tokens) != 0) {
// 		return -1;
// 	}
// 	if (tokens.empty()) {
// 		return -1;
// 	}
// 	return 0;
// }


// int ConfigParser::LeftSideLex(const std::string &expr, std::string &left, std::string &right) const noexcept {
// 	left = "";
// 	right = "";

// 	for (size_t i = 0; i < expr.length(); ++i) {
// 		const char &c = expr[i];

// 		// ignore the blank character
// 		if (c == ' ') {
// 			continue;
// 		}

// 		if (c == '=') {
// 			if (left.empty()) {
// 				std::cerr << "Error: Expected left hand side variable before '='" << std::endl;
// 				return -1;
// 			} else {
// 				right = expr.substr(i+1, expr.length()-i-1);
// 				return 0;
// 			}
// 		} else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
// 			left += c;
// 		} else if (c >= '0' && c <= '9') {
// 			left += c;
// 		} else {
// 			if (left.empty()) {
// 				std::cerr << "Error: Expected expression starts with left hand side variable!" << std::endl;
// 			} else {
// 				std::cerr << "Error: Invalid symbol in left hand side variable: " << c << std::endl;
// 			}
// 			return -1;
// 		}
// 	}
// 	return 0;
// }


// int ConfigParser::ExtendDividerParse(const std::string &left, const std::vector<TokenPtr> &right, size_t divisor) noexcept {
// 	// check identifiers form
// 	if (!CheckIdentifiers(left, right)) {
// 		std::cerr << "Error: Identifiers form!" << std::endl;
// 		return -1;
// 	}
// 	// check port input and output conflict
// 	if (!CheckIoConflict(left, right)) {
// 		std::cerr << "Error: Input output conflict!" << std::endl;
// 		return -1;
// 	}

// 	if (IsDivider(left) && !divisor) {
// 		std::cerr << "Error: Expected \" / divisor\" in divider expression" << std::endl;
// 		return -1;
// 	}

// 	if (right.size() == 1 && !IsClock(right[0]->Name())) {
// 		size_t id_index = IdentifierIndex(right[0]->Name());
// 		if (IsFrontIo(right[0]->Name())) {
// 			front_in_use_.set(id_index);
// 			if (IsLemoIo(right[0]->Name())) {
// 				front_use_lemo_.set(id_index);
// 			}
// 		}
// 		return id_index;
// 	}


// 	// parse the right side tokens and generate the syntax tree
// 	LogicalGrammar grammar;
// 	SLRSyntaxParser<bool> parser(&grammar);

// 	if (IsDivider(right[0]->Name())) {
// 		// contains divider
// 		std::vector<TokenPtr> extracted_tokens(right.begin()+2, right.end());
// 		if (parser.Parse(extracted_tokens) != 0) {
// 			std::cerr << "Error: Syntax parse failed" << std::endl;
// 			return -1;
// 		}
// 	} else {
// 		if (parser.Parse(right) != 0) {
// 			std::cerr << "Error: Syntax parse failed" << std::endl;
// 			return -1;
// 		}
// 	}

// 	// standardize the syntax tree
// 	StandardLogicTree tree(parser.Root());

// 	// generate gates
// 	int generate_index = GenerateGates(tree.Root(), tree.IdList());
// 	if (generate_index < 0) {
// 		std::cerr << "Error: Generate gates!" << std::endl;
// 		return -1;
// 	}

// 	if (IsDivider(right[0]->Name())) {
// 		std::vector<TokenPtr> divider_tokens(right.begin(), right.begin()+2);
// 		generate_index = GenerateDividerGate(divider_tokens, size_t(generate_index));
// 		if (generate_index < 0) {
// 			std::cerr << "Error: Generate divider gate" << std::endl;
// 			return -1;
// 		}
// 	}

// 	return generate_index;
// }


bool ConfigParser::CheckIdentifiers(const std::vector<TokenPtr> &tokens) const noexcept {
	std::string left = tokens[0]->Name();
	// check left side identifier
	if (
		!IsFrontIo(left)
		&& !IsBack(left)
		&& !IsExternalClock(left)
		&& !IsScaler(left)
		&& !IsDivider(left)
	) {
		return false;
	}
	// check right side identifier
	if (tokens.size() == 3) {
		if (tokens[2]->Type() != kSymbolType_Variable) {
			return false;
		}
		if (IsClock(tokens[2]->Name())) {
			if (!IsFrontIo(left) && !IsExternalClock(left)) {
				// if right is clock, left must be front io port
				// or external clock port
				return false;
			}
		} else if (!IsFrontIo(tokens[2]->Name())) {
			return false;
		}
	} else {
		for (size_t i = 2; i < tokens.size(); ++i) {
			auto &id = tokens[i];
			if (id->Type() == kSymbolType_Variable) {
				if (!IsFrontIo(id->Name())) {
					std::cerr << "Error: Expected identifier in front io port form "
						<< id->Name() << "\n";
					return false;
				}
			} else if (
				id->Type() != kSymbolType_Operator
				&& id->Type() != kSymbolType_Literal
			) {
				std::cerr << "Error: Invalid identifier type "
					<< id->Type() << "\n";
				return false;
			}
		}
	}
	return true;
}



bool ConfigParser::CheckIoConflict(const std::vector<TokenPtr> &tokens) const noexcept {
	std::string left = tokens[0]->Name();
	// check output conflict, i.e. an output port with two sources
	if (IsBack(left)) {
		// check back output conflict
		if (back_output_ != size_t(-1)) {
			// two source of back
			std::cerr << "Error: Multiple source of back plane port.\n";
			return false;
		}
	} else if (IsExternalClock(left)) {
		// check external clock output conflict
		if (extern_clock_ != size_t(-1)) {
			// two source of external clock
			std::cerr << "Error: Multiple source of external clock.\n";
			return false;
		}
	} else if (IsFrontIo(left)) {
		// check front output port conflict
		if (front_out_use_.test(IdentifierIndex(left))) {
			// front output conflict
			std::cerr << "Error: Multiple source of fornt port " << left << std::endl;
			return false;
		}
	} else if (IsScaler(left)) {
		// check scaler source conflict
		if (scaler_use_.test(IdentifierIndex(left)-kScalersOffset)) {
			// scaler source conflict
			std::cerr << "Error: Multiple source of scaler " << left << std::endl;
			return false;
		}
	}

	// check input output conflict, i.e. a port is both an input and output port
	// check itself consistent, expect no port both in left and right
	if (IsFrontIo(tokens[2]->Name())) {
		// not the clock
		for (size_t i = 2; i < tokens.size(); ++i) {
			if (tokens[i]->Type() != kSymbolType_Variable) {
				continue;
			}
			if (tokens[i]->Name() == left) {
				std::cerr << "Error: Input and output in the same port "
					<< left << ".\n";
				return false;
			}
		}
	}

	// check this output with previous inputs
	if (IsFrontIo(left) && front_in_use_.test(IdentifierIndex(left))) {
		std::cerr << "Error: Output port defined as input port before "
			<< left << ".\n";
		return false;
	}
	// check this inputs with previous outputs
	if (!IsClock(tokens[2]->Name())) {
		// not the clock
		if (IsScaler(left) && tokens.size() == 3) {
			if (tokens[2]->Type() != kSymbolType_Variable) {
				std::cerr << "Error: The only token is not identifier "
					<< tokens[2]->Name() << ".\n";
				return false;
			}
		} else {
			for (size_t i = 2; i < tokens.size(); ++i) {
				if (tokens[i]->Type() != kSymbolType_Variable) {
					// ignore operators
					continue;
				}
				if (!IsFrontIo(tokens[i]->Name())) {
					// ignore others
					continue;
				}
				if (front_out_use_.test(IdentifierIndex(tokens[i]->Name()))) {
					std::cerr << "Error: Input port defined as output port before "
						<< tokens[i]->Name() << ".\n";
					return false;
				}
			}
		}
	}

	// check lemo input conflict, i.e. an input port defined as both lemo and not
	for (size_t i = 2; i < tokens.size(); ++i) {
		if (tokens[i]->Type() != kSymbolType_Variable) {
			continue;
		}
		if (IsFrontIo(tokens[i]->Name())) {
			if (!front_in_use_.test(IdentifierIndex(tokens[i]->Name()))) {
				continue;
			}
			if (front_use_lemo_.test(IdentifierIndex(tokens[i]->Name()))) {
				if (!IsLemoIo(tokens[i]->Name())) {
					std::cerr << "Error: Input port was defined as lemo input before "
						<< tokens[i]->Name() << ".\n";
					return false;
				}
			} else {
				if (IsLemoIo(tokens[i]->Name())) {
					std::cerr << "Error: Input port wasn't defined as lemo input before "
						<< tokens[i]->Name() << ".\n";
					return false;
				}
			}
		}
	}

	return true;
}



bool ConfigParser::IsFrontIo(const std::string &name) const noexcept {
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


bool ConfigParser::IsLemoIo(const std::string &name) const noexcept {
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


bool ConfigParser::IsClock(const std::string &name) const noexcept {
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


bool ConfigParser::IsScaler(const std::string &name) const noexcept {
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


bool ConfigParser::IsDivider(const std::string &name) const noexcept {
	if (name.substr(0, 2) != "_D") return false;
	for (size_t i = 2; i < name.length(); ++i) {
		if (name[i] < '0' || name[i] > '9') {
			return false;
		}
	}
	std::stringstream ss;
	ss << name.substr(2);
	size_t index;
	ss >> index;
	if (index >= kMaxDividers) return false;
	return true;
}



size_t ConfigParser::IdentifierIndex(const std::string &id) const noexcept {
	if (IsBack(id)) return kBackOffset;
	if (IsExternalClock(id)) return kExternClocksOffset;

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
	} else if (IsDivider(id)) {
		// is divider
		result = atoi(id.substr(2).c_str());
		result += kDividersOffset;
	} else {
		return size_t(-1);
	}

	return result;
}


int ConfigParser::GenerateDivider(
	StandardLogicDownscaleTree *tree,
	StandardLogicNode *node,
	const int divisor
) noexcept {
	int source_index = -1;
	if (node->OperatorType() == kOperatorNull) {
		GatesInfo<kAndBits> gate_info{and_gates_, kAndGatesOffset, kMaxAndGates};
		source_index = GenerateGate(tree, node, 0, gate_info);
	} else if (node->OperatorType() == kOperatorOr) {
		GatesInfo<kOrBits> gate_info{or_gates_, kOrGatesOffset, kMaxOrGates};
		source_index = GenerateGate(tree, node, 1, gate_info);
	} else if (node->OperatorType() == kOperatorAnd) {
		GatesInfo<kAndBits> gate_info{and_gates_, kAndGatesOffset, kMaxAndGates};
		source_index = GenerateGate(tree, node, 2, gate_info);
	}
	DividerInfo info{source_index, divisor};
	// check existence
	for (size_t i = 0; i < dividers_.size(); ++i) {
		if (
			dividers_[i].source == source_index
			&& dividers_[i].divisor == divisor
		) {
			return kDividersOffset + i;
		}
	}
	// not exist
	if (dividers_.size() < kMaxDividers) {
		dividers_.push_back(info);
		return kDividersOffset + dividers_.size() - 1;
	}
}



int ConfigParser::GenerateClock(const std::string &id) noexcept {
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


size_t ConfigParser::ParseFrequency(const std::string &clock) const noexcept {
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


}	 			// namespace ecl