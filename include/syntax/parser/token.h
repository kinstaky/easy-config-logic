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
constexpr int kSymbolType_Operator = 1;
constexpr int kSymbolType_Literal = 2;
constexpr int kSymbolType_Variable = 3;

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
	/// @param[in] name name of the token
	/// @param[in] position position of the token
	/// @param[in] size size of the token
	/// @exceptsafe Shall not throw exceptions.
	///
	Token(
		int type,
		const std::string &name,
		size_t position,
		size_t size
	) noexcept;



	/// @brief default destructor
	///
	virtual ~Token() = default;


	/// @brief name of this token
	/// @returns the token name
	/// @exceptsafe Shall not throw exceptions.
	///
	inline std::string Name() const noexcept {
		return name_;
	}


	/// @brief position in line
	/// @returns position
	///
	inline size_t Position() const noexcept {
		return position_;
	}


	/// @brief size of token
	/// @returns size
	///
	inline size_t Size() const noexcept {
		return size_;
	}

private:
	// string value of this token
	std::string name_;
	// position of this token
	size_t position_;
	// size of this token
	size_t size_;
};


typedef std::shared_ptr<Token> TokenPtr;


/**
 * Variable token
 *
 */
class Variable final : public Token {
public:

	/// @brief constructor
	/// @param[in] name the name of this Variable
	/// @param[in] position position of this variable
	/// @param[in] size size of this variable
	/// @exceptsafe Shall not throw exceptions.
	///
	Variable(
		const std::string &name,
		size_t position = 0,
		size_t size = 0
	) noexcept;


	/// @brief constructor from char
	/// @param[in] name the name of this Varaible
	/// @param[in] position the position of the Varaible
	/// @param[in] size the size of the Varaible
	/// @overload
	/// @exceptsafe Shall not throw exceptions.
	///
	Variable(
		char name,
		size_t positon = 0,
		size_t size = 0
	) noexcept;


	/// @brief default destructor
	///
	virtual ~Variable() = default;


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
	/// @param[in] name the string name of this Operator
	/// @param[in] position position of this Operator
	/// @exceptsafe shall not throw exceptions
	///
	Operator(const std::string &name, size_t position = 0) noexcept;


	/// @brief constructor from char
	///
	/// @param[in] name the char name of this Operator
	///
	/// @overload
	///
	/// @exceptsafe Shall not throw exceptions.
	Operator(char name, size_t position = 0) noexcept;



	/// @brief default destructor
	///
	virtual ~Operator() = default;
};


class NumberLiteral final : public Token {
public:

	/// @brief constructor from integer value
	/// @param[in] value number literal value
	/// @param[in] position position of the literal
	/// @param[in] size size of the literal
	///
	NumberLiteral(
		int value,
		size_t position = 0,
		size_t size = 0
	) noexcept;


	/// @brief default destructor
	///
	virtual ~NumberLiteral() = default;


	/// @brief get literal value
	/// @returns value
	///
	inline virtual int Value() const noexcept {
		return value_;
	}
private:
	int value_;
};

}				// namespace ecl

#endif /* __TOKEN_H__ */