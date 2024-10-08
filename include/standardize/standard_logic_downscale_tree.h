#ifndef __STANDARD_LOGIC_DOWNSCALE_TREE_H__
#define __STANDARD_LOGIC_DOWNSCALE_TREE_H__

#include "syntax/parser/production.h"
#include "standardize/standard_logic_node.h"

namespace ecl {

class StandardLogicDownscaleTree {
public:

	/// @brief constructor
	/// @param[in] production production to convert
	///
	StandardLogicDownscaleTree(Production<int> *production) noexcept;

	/// @brief evaluate the literal leaves and simplify the tree
	/// @returns -1 if nothing changed, 0(1) if get constant value 0(1)
	/// 	2 if the node contains only one leaf
	///
	int EvaluateLiteral(StandardLogicNode *node) noexcept;


	/// @brief standardize the tree
	///
	void Standardize() noexcept;


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
	inline int Divisor(int index) const noexcept {
		return index >= int(divisor_.size()) ? -1 : divisor_[index];
	}


	/// @brief get the variable list
	/// @returns variable list
	///
	inline std::vector<Variable*> VarList() const noexcept {
		return var_list_;
	}


	/// @brief print standard-downscale tree in string format
	/// @param[in] os ostream
	/// @param[in] node current printed node, ignore it in top level
	///
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


	/// @brief get depth of node, consider the downscale
	/// @param[in] node node to get depth
	/// @returns depth, 0 for leaves, 1 for node only with leaves, etc...
	///
	int Depth(StandardLogicNode *node) const noexcept;

private:
	// root node of master tree
	StandardLogicNode *tree_root_;
	// root nodes of extend(downscale) tree
	std::vector<StandardLogicNode*> downscale_forest_;
	std::vector<int> divisor_;
	// identifier list
	std::vector<Variable*> var_list_;


	/// @brief parse production E
	/// @param[in] node current processing node
	/// @param[in] production current processing production
	///
	void ParseE(
		StandardLogicNode *node,
		Production<int> *production
	) noexcept;


	/// @brief parse production T
	/// @param[in] node current processing node
	/// @param[in] production current processing production
	///
	void ParseT(
		StandardLogicNode *node,
		Production<int> *production
	) noexcept;


	/// @brief parse production F
	/// @param[in] node current processing node
	/// @param[in] production current processing production
	///
	void ParseF(
		StandardLogicNode *node,
		Production<int> *production
	) noexcept;

};

}	// namespace ecl


#endif	// __STANDARD_LOGIC_DOWNSCALE_TREE_H__