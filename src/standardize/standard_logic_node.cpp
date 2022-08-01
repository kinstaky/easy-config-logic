#include "standardize/standard_logic_node.h"

#include <iostream>

namespace ecc {


StandardLogicNode::StandardLogicNode(StandardLogicNode *parent, int type) noexcept
:parent_(parent), op_type_(type), leaves_(0) {
}


bool StandardLogicNode::operator==(const StandardLogicNode &node) const noexcept {
	// compare leaves
	if (leaves_ != node.leaves_) return false;
	// compare branches
	if (branches_.size() != node.branches_.size()) return false;
	for (size_t i = 0; i != branches_.size(); ++i) {
		if (!(*(branches_[i]) == *(node.branches_[i]))) {
			return false;
		}
	}
	return true;
}



bool StandardLogicNode::IsOneLeaf(size_t &index) const noexcept {
	if (Depth() == 1) {
		if (LeafSize() == 1) {
			for (size_t i = 0; i < kMaxIdentifier; ++i) {
				if (leaves_.test(i)) {
					index = i;
					break;
				}
			}
			return true;
		}
	} else {
		if (BranchSize() == 1) {
			return branches_[0]->IsOneLeaf(index);
		}
	}
	return false;
}



int StandardLogicNode::AddBranch(StandardLogicNode *node) noexcept {
	if (!AddBranchNecessary(node)) {
		return -1;
	}
	branches_.push_back(node);
	while (true) {
		bool change = false;
		for (size_t i = 0; i < branches_.size()-1; ++i) {
			if (!BranchNecessary(i, node)) {
				branches_[i]->FreeChildren();
				delete branches_[i];
				DeleteBranch(i);
				change = true;
			}
		}
		if (!change) break;
	}
	return 0;
}


bool StandardLogicNode::AddBranchNecessary(StandardLogicNode *node) const noexcept {
	if (node->Depth() == 1 && (node->Leaves() & leaves_).any()) {
		// this branch contains any identifier in node's leaves, so it's unnecessary
		return false;
	}

	for (auto b : branches_) {
		if (*node == *b) {
			return false;
		}
		if ((node->Depth() == 1) && (b->Depth() == 1) && ((b->Leaves() | node->Leaves()) == node->Leaves())) {
			// this branch can contain another branch, it's not necessary
			return false;
		}
	}
	return true;
}


bool StandardLogicNode::BranchNecessary(size_t index, StandardLogicNode *node) const noexcept {
	if (!node && (branches_[index]->Leaves() & leaves_).any()) {
		return false;
	}
	if (node && (node->Depth() == 1) && ((node->Leaves() | branches_[index]->Leaves()) == branches_[index]->Leaves())) {
		return false;
	}
	return true;
} 



int StandardLogicNode::Depth() const noexcept {
	int max_depth = -1;
	if (branches_.size()) {
		for (size_t i = 0; i < branches_.size(); ++i) {
			max_depth = branches_[i]->Depth() > max_depth ? branches_[i]->Depth() : max_depth;
		}
	}
	if (max_depth > 0) return max_depth+1;
	return 1;
}



void StandardLogicNode::PrintString(std::ostream &os, std::vector<Identifier*> id_list) const noexcept {
	std::string op = op_type_ == kOperatorAnd ? " & " : " | ";
	if (branches_.size()) {
		os << std::string("(");
		branches_[0]->PrintString(os, id_list);
		os << std::string(")");
		for (size_t i = 1; i < branches_.size(); ++i) {
			os << op + "(";
			branches_[i]->PrintString(os, id_list);
			os << std::string(")");
		}
	}
	if (leaves_.any()) {
		bool is_first = false;
		if (!branches_.size()) {
			is_first = true;
		}
		for (size_t i = 0; i < id_list.size(); ++i) {
			if (leaves_.test(i)) {
				if (!is_first) {
					os << op << id_list[i]->Value();
				} else {
					os << id_list[i]->Value();
					is_first = false;
				}
			}
		}
	}
	return;
}


void StandardLogicNode::PrintTree(std::vector<Identifier*> id_list, std::string prefix) const noexcept {
	const std::string kBoxHorizental = "\u2500";
	const std::string kBoxVertical = "\u2502";
	const std::string kBoxUpRight = "\u2514";
	const std::string kBoxVerticalRight = "\u251C";


	// print the operator string first
	std::cout << prefix << OperatorString() << std::endl;
	
	std::string children_prefix = "";
	for (size_t i = 0; i < prefix.length(); ++i) {
		if (prefix[i] == ' ') {
			// children prefix is space is the parent prefix is space
			children_prefix += ' ';
		} else if (prefix.substr(i, 3) == kBoxVerticalRight || prefix.substr(i, 3) == kBoxVertical) {
			// children prefix is vertical line if its parent conatins vertical line
			children_prefix += kBoxVertical;
			i += 2;
		} else {
			// children prefix is space if last line is up right or horizental line
			children_prefix += ' ';
			i += 2;
		}
	}

	// print branches
	if (branches_.size()) {
		for (size_t i = 0; i < branches_.size()-1; ++i) {
			branches_[i]->PrintTree(id_list, children_prefix + kBoxVerticalRight + kBoxHorizental);
		}

		if (leaves_.count()) {
			// print last branch
			branches_[branches_.size()-1]->PrintTree(id_list, children_prefix + kBoxVerticalRight + kBoxHorizental);
		} else {
			branches_[branches_.size()-1]->PrintTree(id_list, children_prefix + kBoxUpRight + kBoxHorizental);
		}
	}
	// print leaves
	size_t printed_leaves = 0;
	for (size_t i = 0; i < id_list.size(); ++i) {
		if (leaves_.test(i)) {
			if (printed_leaves == leaves_.count()-1) {
				std::cout << children_prefix << kBoxUpRight << kBoxHorizental << id_list[i]->Value() << std::endl;
				break;
			} else {
				std::cout << children_prefix << kBoxVerticalRight << kBoxHorizental << id_list[i]->Value() << std::endl;
				++printed_leaves;
			}
		}
	}
}


}				// namespace ecc