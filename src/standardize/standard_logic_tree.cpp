#include "standardize/standard_logic_tree.h"

#include <iostream>

#include "syntax/parser/production.h"

namespace ecl {

StandardLogicTree::StandardLogicTree(Production<bool> *production) noexcept {
	// suppose that left hand side of the production is E, and the production
	// shall be one of the following productions:
	//  E -> E | T
	//  E -> E & T
	//  E -> T
	
	Production<bool> *p;
	// loop and find first production E that produce E|T or E&T
	for (p = production; p->size() == 1;) {
		// since p's children size is 1, p is production 3, E -> T
		p = (Production<bool>*)p->Child(0);
		// now p is production 4 or 5, i.e. T -> ( E ) or T -> id
		if (p->size() == 1) {
			// p is production 5, and this tree contains only 1 identifier
			id_list_.push_back((Variable*)p->Child(0));
			tree_root_ = new StandardLogicNode(nullptr, kOperatorNull);
			tree_root_->AddLeaf(0);
			// finish, get out of the loop
			break;		 
		} else {

			// p is production 4, just ignore the bracket
			p = (Production<bool>*)p->Child(1);
		}
	}

	// continue to parse E -> E | T or E -> E & T
	if (p->size() == 3) {
		int op_type = ((Operator*)(p->Child(1)))->Value() == "|" ? kOperatorOr : kOperatorAnd;
		tree_root_ = new StandardLogicNode(nullptr, op_type);

		ParseE(tree_root_, (Production<bool>*)p->Child(0));
		ParseT(tree_root_, (Production<bool>*)p->Child(2));
	}


	// standardize
	if (Standardize() != 0) {
		std::cerr << "standardize error" << std::endl;
		exit(-1);
	}


	// pretty expression
	for (size_t i = 0; i < tree_root_->BranchSize(); ++i) {
		size_t leaf_index;
		if (tree_root_->Branch(i)->IsOneLeaf(leaf_index)) {
			// free old branch
			tree_root_->Branch(i)->FreeChildren();
			delete tree_root_->Branch(i);
			tree_root_->DeleteBranch(i);
			tree_root_->AddLeaf(leaf_index);
		}
	}
}



int StandardLogicTree::ParseE(StandardLogicNode *node, Production<bool> *production) noexcept {
	// production is one of the 3 following productions
	//  1. E -> E | T
	//  2. E -> E & T
	//  3. E -> T
	
	if (production->size() == 3) {
		// production 1 or 2
		int op_type = ((Operator*)production->Child(1))->Value() == "|" ? kOperatorOr : kOperatorAnd;
		if (op_type == node->OperatorType()) {
			// opeartor type is the same with node's, parse it E and T under 
			// the same node
			ParseE(node, (Production<bool>*)production->Child(0));
			ParseT(node, (Production<bool>*)production->Child(2));
		} else {
			// opeartor type is different to the node's, parse E and T under
			// new branch
			StandardLogicNode *new_branch = new StandardLogicNode(node, op_type);
			ParseE(new_branch, (Production<bool>*)production->Child(0));
			ParseT(new_branch, (Production<bool>*)production->Child(2));
			node->AddBranch(new_branch);
		}
	} else {
		// production 3
		ParseT(node, (Production<bool>*)production->Child(0));
	}
	return 0;
}


int StandardLogicTree::ParseT(StandardLogicNode *node, Production<bool> *production) noexcept {
	// production is one of the 2 following productions
	//  4. T -> ( E )
	//  5. T -> id
	if (production->size() == 3) {
		// production 4
		ParseE(node, (Production<bool>*)production->Child(1));
	} else {
		// production 5
		Variable *id = (Variable*)production->Child(0);
		// search whether this identifier appear before
		int index = -1;
		for (size_t i = 0; i < id_list_.size(); ++i) {
			if (id_list_[i] == id) {
				// found the identifier
				index = i;
				break;
			}
		}
		if (index < 0) {
			// identifier not found, add it to the id_list_
			index = id_list_.size();
			id_list_.push_back(id);
		}
		// add identifier as a leaf
		node->AddLeaf(index);
	}
	return 0;
}



int StandardLogicTree::Standardize() noexcept {
	if (ReduceLayers(tree_root_) != 0) {
		return -1;
	}

	if (tree_root_->Depth() == 2 && tree_root_->OperatorType() == kOperatorOr) {
		StandardLogicNode *new_root = ExchangeOrder(tree_root_);
		if (!new_root) {
			return -1;
		}
		tree_root_->FreeChildren();
		delete tree_root_;
		tree_root_ = new_root;
	}
	return 0;
}



int StandardLogicTree::ReduceLayers(StandardLogicNode *node) noexcept {
	// depth under 2, no need to reduce layers
	if (node->Depth() <= 2) {
		return 0;
	}
	// depth over 3, reduce its branches first
	if (node->Depth() > 3) {
		for (size_t i = 0; i < node->BranchSize(); ++i) {
			if (ReduceLayers(node->Branch(i)) != 0) {
				return -1;
			}
		}
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
		for (size_t i = 0; i < node->BranchSize(); ++i) {
			if (node->Branch(i)->Depth() == 2) {
				// first step

				StandardLogicNode *new_branch = ExchangeOrder(node->Branch(i));

				// free old branch
				node->Branch(i)->FreeChildren();
				delete node->Branch(i);
				node->DeleteBranch(i);

				// second step
				// suppose that the new branch only contains depth one branches
				for (size_t j = 0; j < new_branch->BranchSize(); ++j) {
					if (node->AddBranch(new_branch->Branch(j)) != 0) {
						return -1;
					}
					new_branch->Branch(j)->SetParent(node);
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


StandardLogicNode* StandardLogicTree::ExchangeOrder(StandardLogicNode *node) noexcept {
	// new node
	StandardLogicNode *new_node = new StandardLogicNode(node->Parent(), node->Branch(0)->OperatorType());

	// suppose that this node's depth is 2, and it has at least one branch
	std::bitset<kMaxIdentifier> public_id = node->Branch(0)->Leaves();
	std::bitset<kMaxIdentifier> new_leaves = 0;
	StandardLogicNode *prev_node = new StandardLogicNode(nullptr, node->Branch(0)->OperatorType());
	
	// loop node's branches
	for (size_t b = 1; b < node->BranchSize(); ++b) {
		// calculate the new public identifiers
		std::bitset<kMaxIdentifier> new_public_id = public_id & node->Branch(b)->Leaves();
		// residual old public identifiers
		prev_node->AddLeaves(public_id ^ new_public_id);
		// residual new leaves
		new_leaves = node->Branch(b)->Leaves() ^ new_public_id;


		// generate new node
		StandardLogicNode *new_node = new StandardLogicNode(node->Parent(), node->Branch(0)->OperatorType());
		
		// loop the new leaves
		for (size_t i = 0, loop_leaves = 0; i < id_list_.size() && loop_leaves < new_leaves.count(); ++i) {
			if (!new_leaves.test(i)) continue;
			// loop previous node branches
			for (size_t j = 0; j < prev_node->BranchSize(); ++j) {
				// generate new branch
				StandardLogicNode *new_branch = new StandardLogicNode(new_node, node->OperatorType());
				new_branch->AddLeaves(prev_node->Branch(j)->Leaves());
				new_branch->AddLeaf(i);
				new_node->AddBranch(new_branch);
			}
			// loop previous node leaves
			for (size_t j = 0, loop_prev_leaves = 0; j < id_list_.size() && loop_prev_leaves < prev_node->Leaves().count(); ++j) {
				if (!prev_node->Leaves().test(j)) {
					continue;
				}
				// generate new branch
				StandardLogicNode *new_branch = new StandardLogicNode(new_node, node->OperatorType());
				new_branch->AddLeaf(i);
				new_branch->AddLeaf(j);
				new_node->AddBranch(new_branch);

				++loop_prev_leaves;
			} 
			
			++loop_leaves;
		}

		// free previous node
		prev_node->FreeChildren();
		delete prev_node;
		// upate
		public_id = new_public_id;
		prev_node = new_node;

	}

	// loop node's leaves
	for (size_t i = 0, loop_node_leaves = 0; i < id_list_.size() && loop_node_leaves < node->Leaves().count(); ++i) {
		if (!node->Leaves().test(i)) {
			continue;
		}

		// calculate the new public identifiers
		std::bitset<kMaxIdentifier> new_public_id = 0;
		if (!public_id.test(i)) {
			// residual old public identifiers
			prev_node->AddLeaves(public_id);
			
			StandardLogicNode *new_node = new StandardLogicNode(node->Parent(), node->Branch(0)->OperatorType());

			// loop the previous node branches
			for (size_t j = 0; j < prev_node->BranchSize(); ++j) {
				// generate new branches
				StandardLogicNode *new_branch = new StandardLogicNode(new_node, node->OperatorType());
				new_branch->AddLeaves(prev_node->Branch(j)->Leaves());
				new_branch->AddLeaf(i);
				new_node->AddBranch(new_branch);
			}
			// loop previous node leaves
			for (size_t j = 0, loop_prev_leaves = 0; j < id_list_.size() && loop_prev_leaves < prev_node->Leaves().count(); ++j) {
				if (!prev_node->Leaves().test(j)) {
					continue;
				}
				// generate new branch
				StandardLogicNode *new_branch = new StandardLogicNode(new_node, node->OperatorType());
				new_branch->AddLeaf(i);
				new_branch->AddLeaf(j);
				new_node->AddBranch(new_branch);

				++loop_prev_leaves;
			}

			// free previous node
			prev_node->FreeChildren();
			delete prev_node;
			// upate
			public_id = new_public_id;
			prev_node = new_node;
		} else {
			// otherwise, new node should be nothing and has the only public identifier
			new_public_id.set(i);
			// free previous node
			prev_node->FreeChildren();
			delete prev_node;
			//update
			public_id = new_public_id;
			prev_node = new_node;
		}

		
		++loop_node_leaves;
	}


	// add public identifiers as leaves
	prev_node->AddLeaves(public_id);

	return prev_node;
}


std::ostream& operator<<(std::ostream &os, const StandardLogicTree &tree) noexcept {
	tree.tree_root_->PrintString(os, tree.id_list_);
	return os;
}

}
