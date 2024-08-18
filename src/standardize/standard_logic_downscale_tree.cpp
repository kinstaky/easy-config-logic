#include "standardize/standard_logic_downscale_tree.h"

#include <iostream>

namespace ecl {

StandardLogicDownscaleTree::StandardLogicDownscaleTree(
	Production<int> *production
) noexcept {

	// suppose that left hand side of the production is E, and the production
	// shall be one of the following productions:
	//  2. E -> E | T
	//  3. E -> E & T
	//  4. E -> T

	// initalize
	tree_root_ = new StandardLogicNode(nullptr, kOperatorNull);
	downscale_forest_.clear();
	divisor_.clear();

	ParseE(tree_root_, production);

	// standardize master tree
	if (tree_root_->Standardize()) {
		std::cerr << "Error: Standardize master tree failed.\n";
		exit(-1);
	}
	// standardize extend tree
	for (auto &root : downscale_forest_) {
		if (root->Standardize()) {
			std::cerr << "Error: Standardize extend tree failed.\n";
			exit(-1);
		}
	}
}


void StandardLogicDownscaleTree::PrintString(
	std::ostream &os,
	StandardLogicNode *node
) const noexcept {
	if (!node) node = tree_root_;
	bool only_leaf = node->BranchSize() == 0 && node->Leaves().count() == 1;
	// operator string
	std::string op = node->OperatorType() == kOperatorOr ? "|" : "&";
	// print branches
	if (node->BranchSize()) {
		os << "(";
		PrintString(os, node->Branch(0));
		os << ")";
		for (size_t i = 1; i < node->BranchSize(); ++i) {
			os << " " << op + " (";
			PrintString(os, node->Branch(i));
			os << ")";
		}
	}
	// print leaves
	if (node->Leaves().any()) {
		bool is_first = !(node->BranchSize());
		for (size_t i = 0; i < var_list_.size(); ++i) {
			if (!node->Leaf(i)) continue;
			if (!is_first) os << " " << op << " ";
			if (var_list_[i]->Name().substr(0, 2) == "_D") {
				int downscale_index =
					atoi(var_list_[i]->Name().substr(2).c_str());
				if (!only_leaf) os << "(";
				StandardLogicNode *downscale_root =
					downscale_forest_[downscale_index];
				if (downscale_root->BranchSize() || downscale_root->Leaves().count() > 1) {
					os << "(";
				}
				PrintString(os, downscale_forest_[downscale_index]);
				if (downscale_root->BranchSize() || downscale_root->Leaves().count() > 1) {
					os << ")";
				}
				os << " / " << divisor_[downscale_index];
				if (!only_leaf) os << ")";
			} else {
				os << var_list_[i]->Name();
			}
			is_first = false;
		}
	}
}




void StandardLogicDownscaleTree::ParseE(
	StandardLogicNode *node,
	Production<int> *production
) noexcept {
	// production is one of the following
	// 2. E -> E | T
	// 3. E -> E & T
	// 4. E -> T

	if (production->size() == 3) {
		int op_type = ((Operator*)production->Child(1))->Name() == "|"
			? kOperatorOr
			: kOperatorAnd;
		if (node->OperatorType() == kOperatorNull) {
			// node is root
			node->SetOperatorType(op_type);
			ParseE(node, (Production<int>*)production->Child(0));
			ParseT(node, (Production<int>*)production->Child(2));
		} else if (node->OperatorType() == op_type) {
			// same operator, continue on this node
			ParseE(node, (Production<int>*)production->Child(0));
			ParseT(node, (Production<int>*)production->Child(2));
		} else {
			// different operator, add branch
			StandardLogicNode *new_branch = new StandardLogicNode(
				node, op_type
			);
			ParseE(new_branch, (Production<int>*)production->Child(0));
			ParseT(new_branch, (Production<int>*)production->Child(2));
			node->AddBranch(new_branch);
		}
	} else {
		ParseT(node, (Production<int>*)production->Child(0));
	}
}


void StandardLogicDownscaleTree::ParseT(
	StandardLogicNode *node,
	Production<int> *production
) noexcept {
	// production is one of the following
	// 5. T -> F / digits
	// 6. T -> F

	if (production->size() == 1) {
		ParseF(node, (Production<int>*)production->Child(0));
	} else {
		// downscale variable name
		std::string variable_name = "_D" + std::to_string(downscale_forest_.size());
		// variable
		Variable *var = new Variable(variable_name);
		// variable index
		int var_index = var_list_.size();
		// add to variable list
		var_list_.push_back(var);
		// add variable to leaves
		node->AddLeaf(var_index);

		// parse downscale tree
		StandardLogicNode *downscale_tree_root =
			new StandardLogicNode(nullptr, kOperatorNull);
		// add to extend forest
		downscale_forest_.push_back(downscale_tree_root);
		// add divisor
		divisor_.push_back(((NumberLiteral*)production->Child(2))->Value());
		// parse F as downscale tree
		ParseF(downscale_tree_root, (Production<int>*)production->Child(0));
	}
}


void StandardLogicDownscaleTree::ParseF(
	StandardLogicNode *node,
	Production<int> *production
) noexcept {
	// production is one of the following
	// 7. F -> var
	// 8. F -> ( E )

	if (production->size() == 1) {
		Variable *var = (Variable*)production->Child(0);
		// search for this variable
		int index = -1;
		for (size_t i = 0; i < var_list_.size(); ++i) {
			if (var_list_[i] == var) {
				// found
				index = i;
				break;
			}
		}
		if (index < 0) {
			// not found
			index = var_list_.size();
			var_list_.push_back(var);
		}
		// add leaf
		node->AddLeaf(index);
	} else {
		ParseE(node, (Production<int>*)production->Child(1));
	}
}


std::ostream& operator<<(
	std::ostream &os,
	const StandardLogicDownscaleTree &tree
) noexcept {
	tree.PrintString(os);
	return os;
}


} // namespace ecl
