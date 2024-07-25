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
		int op_type = ((Operator*)(p->Child(1)))->Value() == "|"
			? kOperatorOr
			: kOperatorAnd;
		tree_root_ = new StandardLogicNode(nullptr, op_type);

		ParseE(tree_root_, (Production<bool>*)p->Child(0));
		ParseT(tree_root_, (Production<bool>*)p->Child(2));
	}


	// standardize
	if (tree_root_->Standardize()) {
		std::cerr << "Error: Standardize error!\n";
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
		int op_type = ((Operator*)production->Child(1))->Value() == "|"
			? kOperatorOr
			: kOperatorAnd;
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


std::ostream& operator<<(std::ostream &os, const StandardLogicTree &tree) noexcept {
	tree.tree_root_->PrintString(os, tree.id_list_);
	return os;
}

}
