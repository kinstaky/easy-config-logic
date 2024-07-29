#ifndef __STANDARD_LOGIC_DOWNSCALE_TREE_H__
#define __STANDARD_LOGIC_DOWNSCALE_TREE_H__

#include "syntax/parser/production.h"
#include "standardize/standard_logic_node.h"

namespace ecl {

class StandardLogicDownscaleTree {
public:

	StandardLogicDownscaleTree(Production<int> *production) noexcept;


	/// @brief get master tree root node
	/// @returns master tree root node
	///
	inline StandardLogicNode* Root() const noexcept {
		return tree_root_;
	}


	/// @brief get the variable list
	/// @returns variable list
	///
	inline std::vector<Variable*> VarList() const noexcept {
		return var_list_;
	}


	/// @brief print this tree in tree structure
	///
	inline void PrintTree() const noexcept {
		tree_root_->PrintTree(var_list_);
	}


	void PrintString(
		std::ostream &os,
		StandardLogicNode *node = nullptr
	) const noexcept;


	void PrintTree(std::ostream &os) const noexcept;


	/// @brief overload output stream operator function of StandardLogicDownscaleTree
	/// @param[in] os osteam to output
	/// @param[in] tree tree to print
	/// @returns input ostream
	///
	friend std::ostream& operator<<(
		std::ostream &os,
		const StandardLogicDownscaleTree &tree
	) noexcept;

private:
	// root node of master tree
	StandardLogicNode *tree_root_;
	// root nodes of extend(downscale) tree
	std::vector<StandardLogicNode*> downscale_forest_;
	std::vector<int> divisor_;
	// identifier list
	std::vector<Variable*> var_list_;


	void ParseE(
		StandardLogicNode *node,
		Production<int> *production
	) noexcept;

	void ParseT(
		StandardLogicNode *node,
		Production<int> *production
	) noexcept;

	void ParseF(
		StandardLogicNode *node,
		Production<int> *production
	) noexcept;
};

}	// namespace ecl


#endif	// __STANDARD_LOGIC_DOWNSCALE_TREE_H__