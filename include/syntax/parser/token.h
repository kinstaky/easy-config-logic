/*
 * This file defines the terminal tokens in the context-free grammar.
 */

#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <memory>
#include <string>

namespace ecl {

constexpr int kSymbolType_ProductionItem = -4;
constexpr int kSymbolType_ProductionFactorySet = -3;
constexpr int kSymbolType_ProductionFactory = -2;
constexpr int kSymbolType_Production = -1;
constexpr int kSymbolType_Identifier = 1;
constexpr int kSymbolType_Operator = 2;

/**
 * Symbol represents the base element in the context-free grammar.
 * It includes the terminal symbol and the nonterminal symbol. The
 * terminal symbol is represented by class Token and the nonterminal
 * symbol is represented by class Production which is expandable.
 *
 */
class Symbol {
public:

	/// @brief contructor
	///
	/// @param[in] type type of this symbol
	///
	/// @exceptsafe This function should not throw exceptions.
	///
	Symbol(int type) noexcept;


	/// @brief default destructor
	///
	virtual ~Symbol() = default;



	/// @brief returns the symbol type
	///
	/// @returns the symbol type
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline int Type() const noexcept {
		return type_;
	}

private:
	int type_;
};



/**
 * Token represents the terminal symbol in the context-free grammar.
 * It includes the Identifier, Operator and Literal.
 *
 */
class Token: public Symbol {
public:

	/// @brief constructor
	/// @note For now this constructor returns a new instance
	/// 	of Token, but maybe it should return a pointer
	/// 	to the singleton token and update the symbol table.
	///
	/// @param[in] type type of this token
	/// @param[in] value the string value of the token
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	Token(int type, const std::string &value) noexcept;



	/// @brief default destructor
	///
	virtual ~Token() = default;


	/// @brief value of this token
	///
	/// @returns the token value
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline std::string Value() const noexcept {
		return value_;
	}

private:
	std::string value_;					// string value of this token
};


typedef std::shared_ptr<Token> TokenPtr;


/**
 * Identifier token
 *
 */
class Identifier final : public Token {
public:

	/// @brief constructor
	///
	/// @param[in] value the string value of this Identifier
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	Identifier(const std::string &value) noexcept;


	/// @brief constructor from char
	///
	/// @param[in] value the char value of this Operator
	///
	/// @overload
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	Identifier(char value) noexcept;


	/// @brief default destructor
	///
	virtual ~Identifier() = default;


	/// @brief attach variable
	///
	/// @param[in] var_ptr pointer to the attaching variable
	///	@returns 0 on success, -1 on null pointer
	///
	/// @exceptsafe Shall not throw exceptions.
	int Attach(void *var_ptr) noexcept;


	/// @brief attached variable pointer
	///
	/// @returns the attached variable pointer
	///
	/// @exceptsafe Shall not throw exceptions.
	void* GetAttached() const noexcept;

private:
	void *attached_;
};




class Operator final : public Token {
public:

	/// @brief constructor from string
	///
	/// @param[in] value the string value of this Operator
	///
	/// @exceptsafe shall not throw exceptions
	///
	Operator(const std::string &value) noexcept;


	/// @brief constructor from char
	///
	/// @param[in] value the char value of this Operator
	///
	/// @overload
	///
	/// @exceptsafe Shall not throw exceptions.
	Operator(char value) noexcept;



	/// @brief default destructor
	///
	virtual ~Operator() = default;

private:

};

}				// namespace ecl

#endif /* __TOKEN_H__ */