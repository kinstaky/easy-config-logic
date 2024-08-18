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


	/// @brief get downscale tree
	/// @returns downscale tree root nodes
	///
	inline const std::vector<StandardLogicNode*>& Forest() const noexcept {
		return downscale_forest_;
	}


	/// @brief get divisor
	/// @param[in] index divider index
	/// @returns divisor for specified index, -1 for error
	///
	inline const int Divisor(int index) const noexcept {
		return index >= divisor_.size() ? -1 : divisor_[index];
	}


	/// @brief get the variable list
	/// @returns variable list
	///
	inline std::vector<Variable*> VarList() const noexcept {
		return var_list_;
	}


	void PrintString(
		std::ostream &os,
		StandardLogicNode *node = nullptr
	) const noexcept;


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