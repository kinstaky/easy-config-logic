/*
 * This file is part of the StandardizeLogic program.
 */

#ifndef __STANDARD_LOGIC_NODE_H__
#define __STANDARD_LOGIC_NODE_H__

#include <bitset>
#include <vector>
#include <map>


#include "syntax/parser/token.h"

namespace ecc {

constexpr size_t kMaxIdentifier = 64;


constexpr int kOperatorNull = 0;
constexpr int kOperatorOr = 1;
constexpr int kOperatorAnd = 2;

class StandardLogicNode {
public:



	/// @brief Construct a new Standard Logic Node object
	///
	/// @param[in] parent pointer to the parent
	/// @param[in] type operator type of the node
	///
	StandardLogicNode(StandardLogicNode *parent, int type) noexcept;





	/// @brief Destroy the Standard Logic Node object
	///
	~StandardLogicNode() = default;



	/// @brief compare two nodes
	/// 
	/// @param[in] node the node to compare
	/// @returns true if two nodes are the same, false otherwise
	///
	bool operator==(const StandardLogicNode &node) const noexcept;


	/// @brief free children and their children
	/// 
	inline void FreeChildren() noexcept {
		for (StandardLogicNode *node : branches_) {
			node->FreeChildren();
			delete node;
		}
		return;
	}


	//-------------------------------------------------------------------------
	// 							methods for operator
	//-------------------------------------------------------------------------	

	/// @brief get the operator type 
	/// 
	/// @returns 0 on null, 1 on or, 2 on and
	///
	inline int OperatorType() const noexcept {
		return op_type_;
	}


	/// @brief get the operator string 
	/// 
	/// @returns string value of the operator
	///
	inline std::string OperatorString() const noexcept {
		if (op_type_ == kOperatorNull) return "null";
		if (op_type_ == kOperatorOr) return "or";
		if (op_type_ == kOperatorAnd) return "and";
		return "error";
	}


	//-------------------------------------------------------------------------
	// 							methods for parent
	//-------------------------------------------------------------------------	

	/// @brief change the parent node
	/// 
	/// @param[in] node pointer to the parent node
	/// @returns 0 on success, -1 on failure
	///
	inline int SetParent(StandardLogicNode *node) noexcept {
		parent_ = node;
		return 0;
	}


	/// @brief get the parent 
	/// 
	/// @returns pointer to the parent
	///
	inline StandardLogicNode* Parent() const noexcept {
		return parent_;
	}


	//-------------------------------------------------------------------------
	// 							methods for leaves
	//-------------------------------------------------------------------------	

	/// @brief add a leaf by global index 
	/// 
	/// @param[in] index global index of the adding leaf
	/// @returns 0 on success, -1 on invalid index
	///
	inline int AddLeaf(size_t index) noexcept {
		if (index >= kMaxIdentifier) return -1;
		leaves_.set(index);
		for (size_t i = 0; i < branches_.size(); ++i) {
			if (!BranchNecessary(i)) {
				branches_[i]->FreeChildren();
				delete branches_[i];
				DeleteBranch(i);
			}
		}
		return 0;
	}


	/// @brief add several leaves in bitset form 
	/// 
	/// @param[in] leaves addding leaves in bitset form
	/// @returns 0 on success, -1 on failure
	///
	inline int AddLeaves(std::bitset<kMaxIdentifier> leaves) noexcept {
		leaves_ |= leaves;
		return 0;
	}


	/// @brief delete a leaf by global index
	/// 
	/// @param[in] index global index of the deleting leaf
	/// @returns 0 on success, -1 on invalid index, -2 on leaf not exist
	///
	inline int DeleteLeaf(size_t index) noexcept {
		if (index >= kMaxIdentifier) return -1;
		if (!leaves_.test(index)) return -2;
		leaves_.reset(index);
		return 0;
	}


	/// @brief get the leaves size 
	/// 
	/// @returns size of the leaves
	///
	inline size_t LeafSize() const noexcept {
		return leaves_.count();
	}


	/// @brief check whether identifier at index exists in this node 
	/// 
	/// @param[in] index index of the global identifier
	/// @returns true if exists, false otherwise
	///
	inline bool Leaf(size_t index) const noexcept {
		if (index >= kMaxIdentifier) return false;
		return leaves_.test(index);
	}


	/// @brief get the leaves flag 
	/// 
	/// @returns leaves bitset flag
	///
	inline std::bitset<kMaxIdentifier> Leaves() const noexcept {
		return leaves_;
	}


	/// @brief check whether this node is actually a leaf(contains only one identifier)  
	/// 
	/// @param[in] index index of the only identifier if this node is leaf
	/// @returns true if this node is actually a leaf, false otherwises
	///
	bool IsOneLeaf(size_t &index) const noexcept;


	//-------------------------------------------------------------------------
	// 							methods for branches
	//-------------------------------------------------------------------------	

	/// @brief add a branch node 
	/// 
	/// @param[in] node the node to add
	/// @returns 0 on success, -1 on failure
	///
	int AddBranch(StandardLogicNode *node) noexcept;

	/// @brief change the branch by index
	/// 
	/// @param[in] index the index of branch to change
	/// @param[in] node the new branch to replace
	/// @returns 0 on success, -1 on failure
	/// 
	inline int SetBranch(size_t index, StandardLogicNode *node) noexcept {
		if (index >= branches_.size()) return -1;
		branches_[index] = node;
		return 0;
	}


	/// @brief  delete a branch by index
	/// 
	/// @param[in] index the index of the branch to delete
	/// @returns 0 on success, -1 on failure
	///
	inline int DeleteBranch(size_t index) noexcept {
		if (index >= branches_.size()) return -1;
		auto it = branches_.begin();
		std::advance(it, index);
		branches_.erase(it);
		return 0;
	}


	/// @brief get the branch by index 
	/// 
	/// @param[in] index index of the branch to get
	/// @returns pointer to the branch at index, or nullptr otherwise
	///
	inline StandardLogicNode* Branch(size_t index) const noexcept {
		if (index >= branches_.size()) return nullptr;
		return branches_[index];
	}


	/// @brief get the branch size 
	/// 
	/// @returns size of the branches
	///
	inline size_t BranchSize() const noexcept {
		return branches_.size();
	}


	/// @brief check whether is necessary to add this branch 
	/// 
	/// @param[in] node the pointer to the branch to check
	/// @returns true if necessary, false otherwise
	///
	bool AddBranchNecessary(StandardLogicNode *node) const noexcept;


	/// @brief check whether the specific branch is necessary
	/// @note If the node is nullptr, this function compare the branch's leaves
	/// 	at index with the leaves_. Otherwise, this function compare the branches
	/// 	with the node.
	/// 
	/// @param[in] index the index of the branch to check
	/// @param[in] node the branch to compare, default is nullptr and just
	/// 	compare the leaves
	/// @returns true if necessary, false otherwise
	///
	bool BranchNecessary(size_t index, StandardLogicNode *node = nullptr) const noexcept;


	//-------------------------------------------------------------------------
	// 							methods for depth
	//-------------------------------------------------------------------------	

	/// @brief get the depth of this node 
	/// 
	/// @returns 0 if it's leaf, or n+1 for n is the largets depth of all branches
	///
	int Depth() const noexcept;

	

	//-------------------------------------------------------------------------
	// 							methods for printing
	//-------------------------------------------------------------------------	

	/// @brief print the string of this node 
	/// 
	/// @param[in] os ostream
	/// @param[in] id_list list of all identifiers in tree
	///
	void PrintString(std::ostream &os, std::vector<Identifier*> id_list) const noexcept;


	/// @brief print the node in tree structure 
	/// 
	/// @param[in] id_list list of all identifiers in tree
	/// @param[in] prefix prefix string of this node
	///
	void PrintTree(std::vector<Identifier*> id_list, std::string prefix = "") const noexcept;



private:

	StandardLogicNode *parent_;							// pointer to the parent node
	int op_type_;										// operator type
	std::vector<StandardLogicNode*> branches_;			// branches
	std::bitset<kMaxIdentifier> leaves_;				// leaves
};

}					// namespace ecc

#endif 				// __STANDARD_LOGIC_NODE_H__