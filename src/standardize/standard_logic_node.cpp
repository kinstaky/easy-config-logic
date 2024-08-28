#include "standardize/standard_logic_node.h"

#include <iostream>

namespace ecl {


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
			max_depth = branches_[i]->Depth() > max_depth
				? branches_[i]->Depth()
				: max_depth;
		}
	}
	if (max_depth > 0) return max_depth+1;
	return 1;
}



void StandardLogicNode::PrintString(std::ostream &os, std::vector<Variable*> id_list) const noexcept {
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
					os << op << id_list[i]->Name();
				} else {
					os << id_list[i]->Name();
					is_first = false;
				}
			}
		}
	}
	return;
}


void StandardLogicNode::PrintTree(std::vector<Variable*> id_list, std::string prefix) const noexcept {
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
				std::cout << children_prefix << kBoxUpRight << kBoxHorizental
					<< id_list[i]->Name() << std::endl;
				break;
			} else {
				std::cout << children_prefix << kBoxVerticalRight << kBoxHorizental
					<< id_list[i]->Name() << std::endl;
				++printed_leaves;
			}
		}
	}
}


void PrettyNode(StandardLogicNode *node) {
	for (size_t i = 0; i < node->BranchSize(); ++i) {
		PrettyNode(node->Branch(i));
	}

}


int StandardLogicNode::Standardize() noexcept {
	// reduce layers, and get the tree with only 1 or 2 layers
	if (ReduceLayers()) return -1;

	// exchange the only 2 layers if the first layer is '|'
	if (Depth() == 2 && op_type_ == kOperatorOr) {
		StandardLogicNode *new_root = ExchangeOrder();
		if (!new_root) return -1;
		FreeChildren();
		branches_.clear();
		op_type_ = new_root->OperatorType();
		leaves_ = new_root->Leaves();
		for (size_t i = 0; i < new_root->BranchSize(); ++i) {
			if (AddBranch(new_root->Branch(i))) {
				new_root->Branch(i)->FreeChildren();
				delete new_root->Branch(i);
			}
		}
		delete new_root;
	}

	// make it pretty, and remove branch with single leaf
	bool change = true;
	while (change) {
		change = false;
		for (size_t i = 0; i < branches_.size(); ++i) {
			size_t leaf = 0;
			if (branches_[i]->IsOneLeaf(leaf)) {
				leaves_.set(leaf);
				DeleteBranch(i);
				change = true;
				break;
			}
		}
	}

	return 0;
}


int StandardLogicNode::ReduceLayers() noexcept {
	// no need to reduce layers if there is only 1 or 2 layers
	if (Depth() <= 2) return 0;

	// depth over 3, reduce branches' layers first
	for (size_t i = 0; i < branches_.size(); ++i) {
		if (branches_[i]->ReduceLayers()) return -1;
	}

	// Depth equals to 3, reduce the layers from 3 to 2 in two steps: first
	// step is exchanging the operation order of the second and the third
	// layers; the second step is to delete the second layers and regard the
	// nodes in the third layers as the first layers' children.
	// For example, expression ((A & B) | (C & D)) & (E | F) is 3 layers.
	// After the first step, it becomes ((A|C) & (B|C) & (A|D) & (B|D)) & (E|F).
	// After the second step, it becomes (E|F) & (A|C) & (B|C) & (A|D) & (B|D).
	bool change = true;
	while (change) {
		change = false;
		for (size_t i = 0; i < branches_.size(); ++i) {
			if (branches_[i]->Depth() == 2) {
				// first step
				StandardLogicNode *new_branch = branches_[i]->ExchangeOrder();

				// free old branch
				branches_[i]->FreeChildren();
				delete branches_[i];
				DeleteBranch(i);

				// second step
				// suppose that the new branch only contains depth one branches
				for (size_t j = 0; j < new_branch->BranchSize(); ++j) {
					if (AddBranch(new_branch->Branch(j))) {
						new_branch->Branch(j)->FreeChildren();
						delete new_branch->Branch(j);
						return -1;
					}
					new_branch->Branch(j)->SetParent(this);
				}
				// free new branch
				delete new_branch;
				change = true;
				break;
			}
		}
	}

	return 0;

}


StandardLogicNode* StandardLogicNode::ExchangeOrder() noexcept {
	size_t id_size = kMaxIdentifier;

	// suppose that this node's depth is 2, and it has at least one branch
	std::bitset<kMaxIdentifier> public_id = branches_[0]->Leaves();
	std::bitset<kMaxIdentifier> new_leaves = 0;
	StandardLogicNode *prev_node = new StandardLogicNode(nullptr, branches_[0]->OperatorType());

	// loop node's branches
	for (size_t b = 1; b < branches_.size(); ++b) {
		// calculate the new public identifiers
		std::bitset<kMaxIdentifier> new_public_id = public_id & branches_[b]->Leaves();
		// residual old public identifiers
		prev_node->AddLeaves(public_id ^ new_public_id);
		// residual new leaves
		new_leaves = branches_[b]->Leaves() ^ new_public_id;


		// generate new node
		StandardLogicNode *temp_node = new StandardLogicNode(parent_, branches_[0]->OperatorType());

		// loop the new leaves
		for (
			size_t i = 0, loop_leaves = 0;
			i < id_size && loop_leaves < new_leaves.count();
			++i
		) {
			if (!new_leaves.test(i)) continue;
			// loop previous node branches
			for (size_t j = 0; j < prev_node->BranchSize(); ++j) {
				// generate new branch
				StandardLogicNode *new_branch =
					new StandardLogicNode(temp_node, op_type_);
				new_branch->AddLeaves(prev_node->Branch(j)->Leaves());
				new_branch->AddLeaf(i);
				temp_node->AddBranch(new_branch);
			}
			// loop previous node leaves
			for (
				size_t j = 0, loop_prev_leaves = 0;
				j < id_size && loop_prev_leaves < prev_node->Leaves().count();
				++j
			) {
				if (!prev_node->Leaves().test(j)) continue;
				// generate new branch
				StandardLogicNode *new_branch =
					new StandardLogicNode(temp_node, op_type_);
				new_branch->AddLeaf(i);
				new_branch->AddLeaf(j);
				temp_node->AddBranch(new_branch);
				++loop_prev_leaves;
			}

			++loop_leaves;
		}

		// free previous node
		prev_node->FreeChildren();
		delete prev_node;
		// upate
		public_id = new_public_id;
		prev_node = temp_node;

	}

	// loop node's leaves
	for (
		size_t i = 0, loop_node_leaves = 0;
		i < id_size && loop_node_leaves < leaves_.count();
		++i
	) {
		if (!leaves_.test(i)) continue;

		// calculate the new public identifiers
		std::bitset<kMaxIdentifier> new_public_id = 0;
		if (!public_id.test(i)) {
			// residual old public identifiers
			prev_node->AddLeaves(public_id);

			StandardLogicNode *temp_node =
				new StandardLogicNode(parent_, branches_[0]->OperatorType());

			// loop the previous node branches
			for (size_t j = 0; j < prev_node->BranchSize(); ++j) {
				// generate new branches
				StandardLogicNode *new_branch =
					new StandardLogicNode(temp_node, op_type_);
				new_branch->AddLeaves(prev_node->Branch(j)->Leaves());
				new_branch->AddLeaf(i);
				temp_node->AddBranch(new_branch);
			}
			// loop previous node leaves
			for (
				size_t j = 0, loop_prev_leaves = 0;
				j < id_size && loop_prev_leaves < prev_node->Leaves().count();
				++j
			) {
				if (!prev_node->Leaves().test(j)) {
					continue;
				}
				// generate new branch
				StandardLogicNode *new_branch =
					new StandardLogicNode(temp_node, op_type_);
				new_branch->AddLeaf(i);
				new_branch->AddLeaf(j);
				temp_node->AddBranch(new_branch);
				++loop_prev_leaves;
			}

			// free previous node
			prev_node->FreeChildren();
			delete prev_node;
			// update
			public_id = new_public_id;
			prev_node = temp_node;
		} else {
			// otherwise, new node should be nothing and has the only public identifier
			new_public_id.set(i);
			// free previous node
			prev_node->FreeChildren();
			delete prev_node;
			// update
			public_id = new_public_id;
			prev_node = new StandardLogicNode(
				parent_, branches_[0]->OperatorType()
			);
		}


		++loop_node_leaves;
	}

	// add public identifiers as leaves
	prev_node->AddLeaves(public_id);
	return prev_node;
}


}	// namespace ecl