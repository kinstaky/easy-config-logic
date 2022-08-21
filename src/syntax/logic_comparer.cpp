#include "syntax/logic_comparer.h"

#include <iostream>
#include <algorithm>

namespace ecl {

LogicComparer::LogicComparer() noexcept
: parser_{grammar_, grammar_+1}, tree_root_{nullptr, nullptr} {
}


LogicComparer::~LogicComparer() noexcept {
	for (size_t i = 0; i < 2; ++i) {
		if (tree_root_[i]) {
			FreeChildren(tree_root_[i]);
			delete tree_root_[i];
		}
	}
}


bool LogicComparer::Compare(const std::string &line1, const std::string &line2) noexcept {
	std::string line[2] = {line1, line2};
	std::vector<TokenPtr> tokens[2];
	for (size_t i = 0; i < 2; ++i) {
		if (lexer_[i].Analyse(line[i], tokens[i]) < 0) {
			std::cerr << "Error: lexer analyse expression " << i << std::endl;
			return false;
		}


		// parse the token list
		if (parser_[i].Parse(tokens[i]) < 0) {
			std::cerr << "Error: parser parse token list " << i << std::endl;
			return false;
		}
	}

	if (GenerateNodes() != 0) {
		std::cerr << "Error: genrate nodes" << std::endl;
		return false;
	}
	return CompareValues();

}


int LogicComparer::GenerateNodes() {
	for (size_t index = 0; index < 2; ++index) {
		// init the root node
		Production<bool> *production = parser_[index].Root();
		tree_root_[index] = new Node;

		// the root production must be E
		if (ParseE(production, tree_root_[index], 0) != 0) {
			return -1;
		}

		// calculate the id_flag base on its children
		tree_root_[index]->id_flag = tree_root_[index]->children[0]->id_flag;
		if (tree_root_[index]->size == 2) {
			tree_root_[index]->id_flag |= tree_root_[index]->children[1]->id_flag;
		}
	}
	return 0;
}


int LogicComparer::ParseE(Production<bool> *production, struct Node* node, int layer) {
	// there 3 productions produce by E:
	//  1. E -> E | T
	//  2. E -> E & T
	//  3. E -> T
	if (production->size() == 3) {
		// production 1 or 2

		// config the node
		Operator *op = (Operator*)production->Child(1);
		node->op_type = op->Value() == "|" ? kOperatorOr : kOperatorAnd;
		node->size = 2;
		node->id = nullptr;
		node->cache_value = false;
		node->children[0] = new Node;
		node->children[1] = new Node;

		// first child is produced by E
		if (ParseE((Production<bool>*)production->Child(0), node->children[0], layer+1) != 0) {
			return -1;
		}
		// second child is produced by T
		if (ParseT((Production<bool>*)production->Child(2), node->children[1], layer+1) != 0) {
			return -1;
		}

		// calculate identifiers flag
		node->id_flag = node->children[0]->id_flag | node->children[1]->id_flag;
	} else {
		// production 3

		// config the node
		node->op_type = kOperatorNull;
		node->size = 1;
		node->id = nullptr;
		node->cache_value = false;
		node->children[0] = new Node;

		// the first and only child is produced by T
		if (ParseT((Production<bool>*)production->Child(0), node->children[0], layer+1) != 0) {
			return -1;
		}
		
		// calculate identifiers flag
		node->id_flag = node->children[0]->id_flag;
	}
	return 0;
}

int LogicComparer::ParseT(Production<bool>* production, struct Node* node, int layer) {
	// there are 2 productions produced by T
	//  4. T -> ( E )
	//  5. T -> id
	if (production->size() == 3) {
		// production 4

		// config the node
		node->op_type = kOperatorNull;
		node->size = 1;
		node->id = nullptr;
		node->cache_value = false;
		node->children[0] = new Node;

		// the first and only child is produced by E
		if (ParseE((Production<bool>*)production->Child(1), node->children[0], layer+1) != 0) {
			return -1;
		}

		// calculate identifiers flag
		node->id_flag = node->children[0]->id_flag;
	} else {
		// production 5
		
		// config the node
		node->op_type = kOperatorNull;
		node->size = 0;
		Identifier *id = (Identifier*)production->Child(0);
		node->id = id;
		node->cache_value = false;

		// try to find the identifier
		int id_index = -1;
		for (size_t i = 0; i < id_list_.size(); ++i) {
			if (id_list_[i].id->Value() == id->Value()) {
				id_index = id_list_[i].index;
				break;
			}
		}
		if (id_index < 0) {
			// identifier not foound, add it to the list
			IdentifierInfo info;
			info.id = id;
			info.index = id_list_.size();
			info.layer = layer;
			id_index = id_list_.size();
			id_list_.push_back(info);
		} else {
			// found identifier, check the layer
			if (layer > id_list_[id_index].layer) {
				id_list_[id_index].layer = layer;
			}
		}
		
		// calculate the identifier flag
		node->id_flag = 0;
		node->id_flag.set(id_index);
	}
	return 0;
}


bool LogicComparer::CompareValues() {
	// Sort the identifiers by layer and the main idea is that an identifier in
	// lower layer will affect less nodes so that the cache can be used.
	std::sort(id_list_.begin(), id_list_.end(), [](IdentifierInfo x, IdentifierInfo y){
		return x.layer < y.layer;
	});

	// initialize the variables attached to identifiers
	bool *variables = new bool[id_list_.size()];
	for (size_t i = 0; i < id_list_.size(); ++i) {
		variables[i] = 0;
	}

	// attach variables to the identifiers
	for (size_t i = 0; i < id_list_.size(); ++i) {
		if (tree_root_[0]->id_flag.test(id_list_[i].index)) {
			parser_[0].AttachIdentifier(id_list_[i].id->Value(), variables+i);
		}
		if (tree_root_[1]->id_flag.test(id_list_[i].index)) {
			parser_[1].AttachIdentifier(id_list_[i].id->Value(), variables+i);
		}
	}

	// initialize the changed variables flags, set all at first
	std::bitset<kMaxIdentifiers> change_var = -1;
	// change variables in loop and compare value of two expressions
	for (size_t i = 0; i < (1u << id_list_.size()); ++i) {
		// evaluate
		bool eval0 = Evaluate(tree_root_[0], change_var);
		bool eval1 = Evaluate(tree_root_[1], change_var);
		
		if (eval0 != eval1) {
			// std::cerr << "Not equal!!!" << std::endl;
			delete[] variables;
			return false;
		}

		
		// change variables base on gray code
		// gray code in this loop
		size_t gray = i ^ (i>>2);
		// gary code in next loop
		size_t next_gray = (i+1) ^ ((i+1)>>2);
		// the change bit in gray code
		std::bitset<kMaxIdentifiers> change_gray = (gray ^ next_gray) & ((1<<id_list_.size()) - 1);
		// loop to find the index of change bit of gray code
		for (size_t i = 0; i < id_list_.size(); ++i) {
			if (change_gray.test(i)) {
				// found the index of change bit
				variables[i] = !variables[i];
				// set change bit of identifiers
				change_var = 0;
				change_var.set(id_list_[i].index);
				break;
			}
		}

	}


	delete[] variables;
	return true;
}


bool LogicComparer::Evaluate(struct Node *node, std::bitset<kMaxIdentifiers> change) {
	bool result;
	if ((node->id_flag & change).any()) {
		// identifiers under this node have changed
		if (node->op_type == kOperatorNull) {
			if (node->size) {
				// this node is not the end, ask evaluate its child
				result = Evaluate(node->children[0], change);
			} else {
				// this node is the end, evalute the identifier
				result =  *(static_cast<bool*>(node->id->GetAttached()));
			}
		} else if (node->op_type == kOperatorOr) {
			// evalute its children
			result = Evaluate(node->children[0], change) | Evaluate(node->children[1], change);
		} else {
			// evalute its children
			result = Evaluate(node->children[0], change) & Evaluate(node->children[1], change);
		}
		// update cache
		node->cache_value = result;
		return result;
	} else {
		// identifiers under this node havn't change, just return the cache
		return node->cache_value;
	}
}


void LogicComparer::FreeChildren(struct Node *node) {
	for (size_t i = 0; i < node->size; ++i) {
		FreeChildren(node->children[i]);
		delete node->children[i];
	}
}



void LogicComparer::PrintTree(struct Node *node, std::string prefix) {
	// some unicode vertical and horizontal lines
	const std::string kBoxHorizontal = "\u2500";
	const std::string kBoxVertical = "\u2502";
	const std::string kBoxUpRight = "\u2514";
	const std::string kBoxVerticalRight = "\u251C";

	std::string op_str = "null";
	op_str = node->op_type == kOperatorOr ? "|" : op_str;
	op_str = node->op_type == kOperatorAnd ? "&" : op_str;
	std::cout << prefix << op_str << ", " << node->id_flag.to_string().substr(kMaxIdentifiers - id_list_.size(), id_list_.size()) << ", " << node->cache_value << (node->id ? ", "+node->id->Value() : "") << std::endl;
	std::string children_prefix;
	for (size_t i = 0; i < prefix.length(); ++i) {
		if (prefix[i] ==  ' ') {
			children_prefix += " ";
		} else if (prefix.substr(i, 3) == kBoxVertical || prefix.substr(i, 3) == kBoxVerticalRight) {
			children_prefix += kBoxVertical;
			i += 2;
		} else {
			children_prefix += " ";
			i += 2;
		}
	}

	if (node->size == 2) {
		PrintTree(node->children[0], children_prefix + kBoxVerticalRight + kBoxHorizontal);
	}

	if (node->size > 0) {
		PrintTree(node->children[node->size-1], children_prefix + kBoxUpRight + kBoxHorizontal);
	}
	return;
}


}				// namespace ecl