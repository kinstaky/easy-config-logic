#include "config/config_parser.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "syntax/parser/lexer.h"
#include "syntax/parser/syntax_parser.h"
#include "syntax/logic_downscale_grammar.h"

namespace ecl {

const size_t kMaxGates[4] = {
	kMaxOrGates,
	kMaxAndGates,
	kMaxDividerOrGates,
	kMaxDividerAndGates
};
const size_t kGatesOffset[4] = {
	kOrGatesOffset,
	kAndGatesOffset,
	kDividerOrGatesOffset,
	kDividerAndGatesOffset
};


Gate::Gate(
	uint64_t dword0,
	uint64_t dword1,
	uint64_t dword2,
	uint64_t dword3
) {
	data_[0] = dword0;
	data_[1] = dword1;
	data_[2] = dword2;
	data_[3] = dword3;
}


ConfigParser::ConfigParser() {
	Clear();
}


void ConfigParser::Clear() noexcept {
	front_outputs_.clear();
	front_out_use_ = 0;
	front_in_use_ = 0;
	front_use_lemo_ = 0;
	front_output_inverse_ = 0;
	back_output_ = size_t(-1);
	extern_clock_ = size_t(-1);
	for (int i = 0; i < 4; ++i) gates_[i].clear();
	dividers_.clear();
	clocks_.clear();
	clocks_.push_back(1);
	scalers_.clear();
	scaler_use_ = 0;
	variables_.clear();
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
		if (line.empty()) continue;
		if (Parse(line) != 0) {
			std::cerr << "Error: Parse failure " << line << "\n";
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
	if (tokens.size() < 3) return -1;

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

	std::vector<TokenPtr> right_tokens;
	for (size_t i = 2; i < tokens.size(); ++i) {
		right_tokens.push_back(tokens[i]);
	}

	// replace the tokens
	right_tokens = ReplaceVariables(right_tokens);

	// generate new tokens
	tokens.resize(2);
	// TokenPtr left_token = tokens[0];
	// TokenPtr equal_token = tokens[1];
	// tokens.clear();
	// tokens.push_back(left_token);
	// tokens.push_back(equal_token);
	for (size_t i = 0; i < right_tokens.size(); ++i) {
		tokens.push_back(right_tokens[i]);
	}

	// grammar
	LogicDownscaleGrammar grammar;
	// parser
	SLRSyntaxParser<int> parser(&grammar);
	// parse tokens
	if (parser.Parse(tokens)) {
		std::cerr << "Error: Parse failed: " << expr << "\n";
		return -2;
	}
	// check nested downscale
	if (parser.Eval() >= 2) {
		std::cerr << "Error: Unable to parse expression"
			<< " with nested downscale expression." << expr << "\n";
		return -2;
	}

	// left side token name
	std::string left_name = tokens[0]->Name();


	// standardize
	StandardLogicDownscaleTree tree((Production<int>*)(parser.Root()->Child(2)));

	// // for debug and print tree
	// std::cout << "Root:\n";
	// tree.Root()->PrintTree(tree.VarList());
	// for (size_t i = 0; i < tree.Forest().size(); ++i) {
	// 	std::cout << "Forest " << i << ":\n";
	// 	tree.Forest()[i]->PrintTree(tree.VarList());
	// }

	int generate_index = -1;
	bool is_scaler = IsScaler(left_name);
	if (tree.Root()->OperatorType() == kOperatorNull) {
		if (tree.Root()->Leaf(0)) {
			generate_index = kZeroValueOffset;
		} else if (tree.Root()->Leaf(1)) {
			generate_index = kZeroValueOffset;
		} else {
			generate_index = GenerateGate(&tree, tree.Root(), 0, is_scaler);
		}
	} else if (tree.Root()->OperatorType() == kOperatorOr) {
		generate_index = parser.Eval() == 1
			? GenerateGate(&tree, tree.Root(), 3, is_scaler)
			: GenerateGate(&tree, tree.Root(), 1, is_scaler);
	} else if (tree.Root()->OperatorType() == kOperatorAnd) {
		generate_index = parser.Eval() == 1
			? GenerateGate(&tree, tree.Root(), 4, is_scaler)
			: GenerateGate(&tree, tree.Root(), 2, is_scaler);
	}
	if (generate_index < 0) {
		std::cerr << "Error: Generate gates failed.\n";
		return -1;
	}

	// left side token index
	size_t left_index = IdentifierIndex(left_name);
	if (IsFrontIo(left_name)) {
		front_outputs_.push_back(PortSource{left_index, size_t(generate_index)});
		front_out_use_.set(left_index);
		if (!IsClock(tokens[2]->Name())) {
			front_output_inverse_.set(left_index);
			if (
				tree.Root()->Leaf(1)
				&& tree.Root()->OperatorType() == kOperatorNull
			) {
				front_output_inverse_.reset(left_index);
			}
		}
		if (IsLemoIo(left_name)) {
			front_use_lemo_.set(left_index);
		}
	} else if (IsBack(left_name)) {
		back_output_ = generate_index;
	} else if (IsScaler(left_name)) {
		scalers_.push_back(PortSource{
			left_index-kScalersOffset, size_t(generate_index)
		});
		scaler_use_.set(left_index-kScalersOffset);
	} else if (IsExternalClock(left_name)) {
		extern_clock_ = generate_index;
	} else {
		VariableInfo info;
		info.name = left_name;
		for (size_t i = 2; i < tokens.size(); ++i) {
			info.tokens.push_back(tokens[i]);
		}
		variables_.push_back(info);
	}

	return 0;
}


std::vector<TokenPtr> ConfigParser::ReplaceVariables(
	const std::vector<TokenPtr> &tokens
) noexcept {
	std::vector<TokenPtr> result;
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (
			tokens[i]->Type() != kSymbolType_Variable
			|| !IsVariable(tokens[i]->Name())
		) {
			result.push_back(tokens[i]);
			continue;
		}
		for (const auto &var : variables_) {
			if (var.name != tokens[i]->Name()) continue;
			std::vector<TokenPtr> replace_tokens =
				ReplaceVariables(var.tokens);
			// add left bracket
			result.push_back(std::make_shared<Operator>('('));
			for (size_t j = 0; j < replace_tokens.size(); ++j) {
				result.push_back(replace_tokens[j]);
			}
			// add right bracket
			result.push_back(std::make_shared<Operator>(')'));
			break;
		}
	}
	return result;
}


bool ConfigParser::CheckIdentifiers(
	const std::vector<TokenPtr> &tokens
) const noexcept {
	// left side token name
	std::string left = tokens[0]->Name();
	// check left side identifier
	if (tokens.size() < 3) return false;
	// check right side identifier
	if (tokens.size() == 3) {
		if (
			tokens[2]->Type() != kSymbolType_Variable
			&& tokens[2]->Type() != kSymbolType_Literal
		) {
			return false;
		}
		if (IsClock(tokens[2]->Name())) {
			if (!IsFrontIo(left) && !IsExternalClock(left)) {
				// if right is clock, left must be front io port
				// or external clock port
				return false;
			}
		} else if (
			!IsFrontIo(tokens[2]->Name())
			&& !IsDefinedVariable(tokens[2]->Name())
			&& tokens[2]->Name() != "0"
			&& tokens[2]->Name() != "1"
		) {
			return false;
		}
	} else {
		for (size_t i = 2; i < tokens.size(); ++i) {
			auto &id = tokens[i];
			if (id->Type() == kSymbolType_Variable) {
				if (!IsFrontIo(id->Name()) && !IsDefinedVariable(id->Name())) {
					std::cerr << "Error: Expected identifier is in "
						<< "front io port form or is defined variable "
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


bool ConfigParser::CheckIoConflict(
	const std::vector<TokenPtr> &tokens
) const noexcept {
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
	} else {
		// check user defined variable
		// check redefinition
		for (size_t i = 0; i < variables_.size(); ++i) {
			// found redefinition
			if (variables_[i].name == left) return false;
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
		if (tokens[i]->Type() != kSymbolType_Variable) continue;
		if (!IsFrontIo(tokens[i]->Name())) continue;
		if (!front_in_use_.test(IdentifierIndex(tokens[i]->Name()))) continue;
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

	// check variables, variables should be defined before used
	for (size_t i = 2; i < tokens.size(); ++i) {
		if (tokens[i]->Type() != kSymbolType_Variable) continue;
		if (!IsVariable(tokens[i]->Name())) continue;
		bool found = false;
		for (const auto &var : variables_) {
			if (var.name == tokens[i]->Name()) {
				found = true;
				break;
			}
		}
		if (!found) {
			std::cerr << "Error: Variable used but not defined "
				<< tokens[i]->Name() << "\n";
			return false;
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


bool ConfigParser::IsDefinedVariable(const std::string &name) const noexcept {
	if (!IsVariable(name)) return false;
	for (size_t i = 0; i < variables_.size(); ++i) {
		if (name == variables_[i].name) return true;
	}
	return false;
}


size_t ConfigParser::IdentifierIndex(const std::string &id) const noexcept {
	if (IsBack(id)) return kBackOffset;
	if (IsExternalClock(id)) return kExternalClockOffset;

	if (IsClock(id)) {
		size_t frequency = ParseFrequency(id);
		for (size_t i = 0; i < clocks_.size(); ++i) {
			if (frequency == clocks_[i]) return kClocksOffset + i;
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


int ConfigParser::GenerateGate(
	const StandardLogicDownscaleTree* tree,
	const StandardLogicNode *node,
	const int layer,
	const bool is_scaler
) noexcept {
	// get variables
	std::vector<Variable*> var_list = tree->VarList();

	// initialize
	Gate gate;

	// check branches
	for (size_t i = 0; i < node->BranchSize(); ++i) {
		int gate_index = tree->Depth(node->Branch(i)) >= 4
			? GenerateGate(tree, node->Branch(i), 3, is_scaler)
			: GenerateGate(tree, node->Branch(i), 1, is_scaler);
		// check gate index
		if (gate_index < 0) return -1;
		// set bit
		gate.Set(gate_index);
	}

	// check leaves
	// expect gate bits count
	for (size_t i = 2; i < kMaxIdentifier; ++i) {
		if (!node->Leaf(i)) continue;
		if (IsDivider(var_list[i]->Name())) {
			int divider_index = atoi(var_list[i]->Name().substr(2).c_str());
			StandardLogicNode *downscle_node = tree->Forest().at(divider_index);
			int divisor = tree->Divisor(divider_index);
			// check divisor
			if (divisor <= 0) return -1;
			// get gate index
			int gate_index =
				GenerateDivider(tree, downscle_node, divisor, is_scaler);
			// check valid
			if (gate_index < 0) return -1;
			// return index or set gate index
			if (layer == 0) return gate_index;
			else gate.Set(gate_index);
		} else if (IsFrontIo(var_list[i]->Name())) {
			// add this identifier to input used and add to the and-gate
			size_t id_index = IdentifierIndex(var_list[i]->Name());
			// record front IO used
			if (!is_scaler) front_in_use_.set(id_index);
			// record LEMO used
			if (IsLemoIo(var_list[i]->Name())) front_use_lemo_.set(id_index);
			// return index or set gate bit
			if (layer == 0) return id_index;
			else gate.Set(id_index);
		} else if (IsClock(var_list[i]->Name())) {
			if (layer != 0) return -1;
			return GenerateClock(var_list[i]->Name());
		}
	}

	// check existence
	for (size_t i = 0; i < gates_[layer-1].size(); ++i) {
		if (gate == gates_[layer-1][i]) return kGatesOffset[layer-1] + i;
	}
	// not exist add new one
	if (gates_[layer-1].size() < kMaxGates[layer-1]) {
		gates_[layer-1].push_back(gate);
		return kGatesOffset[layer-1] + gates_[layer-1].size() - 1;
	}

	return -1;
}



int ConfigParser::GenerateDivider(
	const StandardLogicDownscaleTree *tree,
	const StandardLogicNode *node,
	const size_t divisor,
	const bool is_scaler
) noexcept {
	if (divisor == 0) return -1;

	int source_index = -1;
	if (node->OperatorType() == kOperatorNull) {
		source_index = GenerateGate(tree, node, 0, is_scaler);
	} else if (node->OperatorType() == kOperatorOr) {
		source_index = GenerateGate(tree, node, 1, is_scaler);
	} else if (node->OperatorType() == kOperatorAnd) {
		source_index = GenerateGate(tree, node, 2, is_scaler);
	}
	DividerInfo info{size_t(source_index), divisor};
	// check existence
	for (size_t i = 0; i < dividers_.size(); ++i) {
		if (
			dividers_[i].source == size_t(source_index)
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
	// error
	return -1;
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