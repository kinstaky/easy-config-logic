/*
 * This file is part of the context-free grammar library.
 */

#ifndef __LEXER_H__
#define __LEXER_H__

#include <memory>
#include <string>
#include <vector>

#include "syntax/parser/token.h"


namespace ecc {

class Lexer {
public:



	/// @brief default constructor
	Lexer() = default;


	/// @brief default destructor
	~Lexer() = default;



	/// @brief analyse a string and get token list
	/// @note this function is a simple lexer for logical expressions. 
	///		It converts the logical expressions to the token list.
	///
	/// @param[in] expr the expression to analyse
	/// @param[out] tokens the token list
	/// @returns 0 on success, -1 on invalid input
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int Analyse(const std::string &expr, std::vector<TokenPtr> &tokens);
};

}				// namespace ecc


#endif /* __LEXER_H__ */