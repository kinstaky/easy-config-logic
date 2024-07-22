#ifndef __LOGIC_COMPARER_H__
#define __LOGIC_COMPARER_H__

#include <vector>
#include <map>
#include <bitset>

#include "syntax/parser/token.h"
#include "syntax/parser/lexer.h"
#include "syntax/parser/syntax_parser.h"
#include "syntax/logical_grammar.h"

namespace ecl {

const int kMaxIdentifiers = 64;

class LogicComparer {
public:

	/// @brief constructor
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	LogicComparer() noexcept;



	/// @brief default destructor
	///
	~LogicComparer() noexcept;


	/// @brief compare two logical expression
	///
	/// @param[in] line1 first logical expression
	/// @param[in] line2 second logical expression
	/// @return true if equal, false not equal
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	bool Compare(const std::string &line1, const std::string &line2) noexcept;


private:

	/// @brief decorates production to cache evaluted value
	/// @note Node is the structure decorates the concrete producition and it
	/// 	store the cache of the evaluated value of the production. The cache
	/// 	is out of dated when the identifiers(represented in id_flag) changed.
	/// @param op_type type of the operator of the production
	/// @param size size of children of this node, or effective nodes of production
	/// @param children children nodes
	/// @param id pointer to the identifier if this node contains identifier
	/// @param cache_value cache value of the node, essential to the optimizing
	/// @param id_falg identifiers under this node and used to check whether the
	/// 	cache value is out of dated
	///
	const int kOperatorNull = 0;
	const int kOperatorOr = 1;
	const int kOperatorAnd = 2;
	struct Node {
		int op_type;
		size_t size;
		struct Node* children[2];
		Variable *id;
		bool cache_value;
		std::bitset<kMaxIdentifiers> id_flag;
	};


	/// @brief generate the tree structre for node
	///
	/// @returns 0 on success, -1 on failure
	///
	int GenerateNodes();


	/// @brief parse the production E and gernerates nodes
	///
	/// @param[in] production concrete production to parse
	/// @param[in] node generate nodes under this node
	/// @param[in] layer layer of node from root
	/// @returns 0 on success, -1 on failure
	///
	int ParseE(Production<bool> *production, struct Node* node, int layer);


	/// @brief parse the production T and generates nodes
	///
	/// @param[in] production concrete production to parse
	/// @param[in] node generate nodes under this node
	/// @param[in] layer layer of node from root
	/// @returns 0 on success, -1 on failure
	///
	int ParseT(Production<bool> *production, struct Node* node, int layer);


	/// @brief compare all values of the two expression when the identifiers value changes
	///
	/// @returns true when two expressions are the same, false otherwise
	///
	bool CompareValues();

	/// @brief evaluate the value of (part of the) expression
	///
	/// @param[in] node evalutes under this node
	/// @param[in] change identifiers change flag
	/// @returns evaluated value in boolean
	///
	bool Evaluate(struct Node *node, std::bitset<kMaxIdentifiers> change);


	/// @brief free node's children pointer
	///
	/// @param[in] node the node to free
	///
	void FreeChildren(struct Node *node);


	/// @brief print the tree structure of node
	///
	/// @param[in] node print and regard this node as root
	/// @param[in] prefix printing prefix of this node
	///
	void PrintTree(struct Node *node, std::string prefix = "");



	Lexer lexer_[2];
	LogicalGrammar grammar_[2];
	SLRSyntaxParser<bool> parser_[2];
	std::vector<bool> is_irrelevant_[2];


	struct Node* tree_root_[2];

	/// @brief store more identifier information in the class LogicComparer
	///
	/// @param id pointer to the identifier
	/// @param index the index of the identifier and it's consistent with the
	/// 	identifier flag in bitset form. But it's different form the index
	/// 	from the id_list_ since the id_list_ will be sorted later.
	/// @param layer the layer count from the root
	///
	struct IdentifierInfo {
		Variable *id;
		size_t index;
		int layer;
	};
	std::vector<IdentifierInfo> id_list_;
	std::vector<bool> results_;
};

}				// namespace ecl

#endif