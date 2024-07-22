/*
 * This file is part of the context-free grammar parser library.
 */

#ifndef __SYNTAX_PARSER_H__
#define __SYNTAX_PARSER_H__

#include <map>
#include <vector>

#include "syntax/parser/grammar.h"
#include "syntax/parser/production.h"
#include "syntax/parser/token.h"

namespace ecl {

/**
 * This struct Action represents the action of the LR syntax parse.
 * @param type the action type:
 *  kTypeError: syntax error
 *  kTypeAccept: accept the expression  
 *  kTypeShift: shift the token to the stack 
 *  kTypeGoto: goto the next collection status
 * 	kTypeReduce: reduce the tokens into a production
 * @param collection just used in type kTypeShift and kTypeGoto, and it
 *  represents the next collection status
 * @param production just used in type kTypeReduce, and it represents the
 *  production to reduce
 * 
 */
struct Action {
	static const int kTypeError = -1;
	static const int kTypeAccept = 0;
	static const int kTypeShift = 1;
	static const int kTypeGoto = 2;
	static const int kTypeReduce = 3;

	int type;
	int collection;
	void *production;
};


/**
 * ActionTable, restores the action in this class. It actually use array to
 * store inside. It get or set the action through two parameters, symbol and
 * collection, i.e. an action is mapped to pair of symbol and collection.  
 * 
 */
class ActionTable {
public:

	/// @brief constructor
	///
	/// @param[in] collection_size size of the collection
	/// @param[in] symbol_size size of the symbol of grammar
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	ActionTable(int collection_size, int symbol_size) noexcept;



	/// @brief destructor
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	~ActionTable() noexcept;


	/// @brief set the content of the action table
	///
	/// @param[in] collection present collection before action 
	/// @param[in] symbol next symbol meets
	/// @param[in] type the action type
	/// @param[in] next the next collection after this action
	/// @returns 0 on success, -1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int SetAction(int collection, int symbol, int type, int next) noexcept;



	/// @brief set the content of the action table
	///
	/// @param[in] collection present collection before action 
	/// @param[in] symbol next symbol meets
	/// @param[in] type the action type
	/// @param[in] production the production to reduce
	/// @returns 0 on success, -1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int SetAction(int collection, int symbol, int type, void *production = nullptr) noexcept;


	/// @brief get the action through collection and symbol
	///
	/// @param[in] collection present collection before action
	/// @param[in] symbol next symbol meets
	/// @returns pointer to the action
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	Action* GetAction(int collection, int symbol) noexcept;

private:
	
	struct Action* table_;
	int collection_size_;
	int symbol_size_;
	int size_;
};



/**
 * The SyntaxParser class is the abstract base class of the syntax parser.
 * The syntax parser is attached with a kind of grammar, and then parse the
 * token list and generate the syntax tree. All the derived classes should
 * override the abstract function Parse().
 * 
 * @tparam VarType the return type of the Evaluate function
 */
template<typename VarType>
class SyntaxParser {
public:


	/// @brief initialize the parser according to the parse method
	///
	/// @param[in] grammar pointer to the parsing grammar
	/// @param[in] method parse method, default is LR(0) and choose this when
	/// 	meet illegal input method
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	SyntaxParser(Grammar<VarType> *grammar);


	/// @brief destructor
	///
	virtual ~SyntaxParser() = default;


	/// @brief parse the tokens and generate the concrete syntax tree
	/// @note Parse the tokens from the lexical parser and generate the concrete
	/// 	syntax tree. The generated syntax tree root will be saved. And this
	/// 	is an abstract method, since thera different parse methods.
	///
	/// @param[in] tokens the token list to parse
	/// @returns 0 on success, -1 on failure
	///
	virtual int Parse(const std::vector<TokenPtr>& tokens) = 0;


	/// @brief search the token and return index
	///
	/// @param[in] token shared pointer to the token 
	/// @returns index of the token, -1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int FindSymbol(TokenPtr token) noexcept;



	/// @brief get the identifier list
	///
	/// @returns the identifier list
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline std::vector<Variable*> IdentifierList() const noexcept {
		return identifier_list_;
	}


	/// @brief attach the variable to the identifiers through name
	/// @note The expression may have some identitfiers and the variables should be
	///		attached to them before evaluating the expression.
	///
	/// @param[in] name name of the identifier
	/// @param[in] var_ptr pointer to the attached variable
	/// 
	/// @returns 0 on success, -1 on variable pointer invalid, -2 on identifier
	/// 	name invalid 
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int AttachIdentifier(const std::string &name, void *var_ptr) noexcept;


	/// @brief attach the variable to the identifiers through index
	/// @note The index of the variable is determined by the order from head
	/// 	to tail in the expression.
	///
	/// @param[in] index index of the identifier
	/// @param[in] var_ptr pointer to the attached variable
	/// @returns 0 on success, -1 on variable pointer invalid, -2 on index invalid
 	///
	/// @overload
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int AttachIdentifier(size_t index, void *var_ptr) noexcept;


	// int AttachIdentifiers(void *var_ptr);

	// template<typename... VarTypes>
	// int AttachIdentifiers(void *var_ptr, VarTypes... ptrs);


	/// @brief evaluate the expression value
	/// @note Evaluate the expression value through action.
	///
	/// @returns the expression value
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline VarType Eval() const noexcept {
		return syntax_tree_root_->Eval();
	}

	// template<typename ArgType>
	// static VarType Evaluate(const std::string &expr, ArgType var_ptr);

	// template<typename... ArgTypes>
	// static VarType Evaluate(const std::string &expr, ArgTypes... var_ptrs);

	
	/// @brief print the tree structure of the concrete syntax tree
	///
	/// @param[in] symbol the symbol to print
	/// @param[in] prefix the prefix string, default is empty string
	/// @returns 0 on success, -1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int PrintTree(Symbol *symbol, std::string prefix = "") const noexcept;


	
	/// @brief get the syntax tree root
	///
	/// @returns pointer to the root production
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline Production<VarType>* Root() const noexcept {
		return syntax_tree_root_;
	}

protected:

	// template<typename... ArgTypes>
	// static void RecursiveAttach(int index, ArgTypes... var_ptrs);


	Grammar<VarType> *grammar_;
	Production<VarType> *syntax_tree_root_;
	std::vector<Symbol*> symbol_list_; 
	std::vector<Variable*> identifier_list_;
};




template<typename VarType>
class SLRSyntaxParser final : public SyntaxParser<VarType> {
public:

	/// @brief constructor
	/// @note This function firstly gernerates the grammar item and collection,
	/// 	and generates the action table secondly.
	///
	/// @param[in] grammar pointer to the parsing grammar
	///
	SLRSyntaxParser(Grammar<VarType> *grammar);


	/// @brief destructor
	///
	/// @exceptsafe Shall not throw exceptions.
	/// 
	virtual ~SLRSyntaxParser() noexcept;


	/// @brief get action table
	///
	/// @returns the pointer to the action table
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline ActionTable* GetActionTable() const {
		return action_table_;
	}



	/// @brief parse the token list and generate symbol table and syntax tree
	/// @note This functions parse the token list and generate the symbol table
	/// 	and the syntax tree. The symbols in the table are in the order 
	/// 	of the token list, and one symbol occupies one slot in the table.
	///     The concrete syntax tree locate in the syntax_tree_root_ and was
	/// 	generated based on the grammar.
	///
	/// @param[in] tokens input token list from lexer
	/// @returns 0 on success, -1 on failure
	///	
	virtual int Parse(const std::vector<TokenPtr> &tokens);

private:

	ActionTable *action_table_;
};

}				// namespace ecl

#endif /* __SYNTAX_PARSER_H__ */