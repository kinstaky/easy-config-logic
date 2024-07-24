/*
 * This file is part of the context-free grammar library.
 */

#ifndef __LEXER_H__
#define __LEXER_H__

#include <memory>
#include <string>
#include <vector>

#include "syntax/parser/token.h"


namespace ecl {

class Lexer {
public:
	/// @brief analyse a string and get token list
	/// @note this function is a simple lexer for logical expressions. 
	///		It converts the logical expressions to the token list.
	/// @param[in] expr the expression to analyse
	/// @param[out] tokens the token list
	/// @returns 0 on success
	/// 	-1 on invalid char
	///		-2 on variable starts with digits
	/// 	-3 on variable starts with underscore '_' 
	/// @exceptsafe Shall not throw exceptions.
	///
	int Analyse(const std::string &expr, std::vector<TokenPtr> &tokens);
};

}				// namespace ecl


#endif /* __LEXER_H__ */