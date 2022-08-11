/*
 * This file is part of the StandardizeLogic program.
 */

#ifndef __STANDARD_LOGIC_TREE_H__
#define __STANDARD_LOGIC_TREE_H__

#include <ostream>

#include "syntax/parser/production.h"

#include "standardize/standard_logic_node.h"

namespace ecl {

class StandardLogicTree {
public:


	/// @brief constructor
	///
	/// @param[in] production root of production tree to standardize
	///
	StandardLogicTree(Production<bool> *production) noexcept;


	/// @brief default destructor
	///
	~StandardLogicTree() = default;


	/// @brief get the tree root
	///
	/// @returns pointer to the root
	///
	inline StandardLogicNode* Root() const noexcept {
		return tree_root_;
	}


	/// @brief get the id list
	///
	/// @returns id list
	///
	inline std::vector<Identifier*> IdList() const noexcept {
		return id_list_;
	}


	/// @brief print this tree in tree structure
	///
	inline void PrintTree() const noexcept {
		tree_root_->PrintTree(id_list_);
	}


	/// @brief overload output stream operator function of StandardLogicTree
	///
	/// @param[in] os osteam to output
	/// @param[in] tree tree to print
	/// @returns input ostream
	///
	friend std::ostream& operator<<(std::ostream &os, const StandardLogicTree &tree) noexcept;



private:

	/// @brief parse production E and generate standard tree
	///
	/// @param[in] node parse the production under this node
	/// @param[in] production pointer to the production to parse
	/// @returns 0 on success, -1 on failure
	///
	int ParseE(StandardLogicNode *node, Production<bool> *production) noexcept;


	/// @brief parse production T and generate standard tree
	///
	/// @param[in] node parse the production under this node
	/// @param[in] production pointer to the production to parse
	/// @returns 0 on success, -1 on failure
	///
	int ParseT(StandardLogicNode *node, Production<bool> *production) noexcept;


	/// @brief standardize the tree and convert it into two layers
	///
	/// @returns 0 on success, -1 on failure
	///
	int Standardize() noexcept;


	int ReduceLayers(StandardLogicNode *node) noexcept;


	/// @brief exchange the order of operation of two layers structure
	///
	/// @param[in] node the node the exchange operation order
	/// @returns pointer to the new node after exchanging
	///
	StandardLogicNode* ExchangeOrder(StandardLogicNode *node) noexcept;


	StandardLogicNode *tree_root_;
	std::vector<Identifier*> id_list_;
};


}					// namespace ecl

#endif 				// __STANDARD_LOGIC_TREE_H__