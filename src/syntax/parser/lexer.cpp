/*
 * This file is part of the context-free grammar library.
 */

#include "syntax/parser/lexer.h"

#include <memory>
#include <string>
#include <vector>

#include "syntax/parser/token.h"

namespace ecl {

ParseResult Lexer::Analyse(
	const std::string &expr,
	std::vector<TokenPtr> &tokens
) {
	// value
	std::string value = "";
	// only digits
	bool only_digits = true;
	// identifier starts with digits
	bool start_with_digits = false;

	for (size_t position = 0; position < expr.size(); ++position) {
		char c = expr[position];

		// ignore the blank character
		if (c == ' ') continue;

		if (c == '(' || c == ')' || c == '&' || c == '|' || c == '=' || c == '/') {
			// the needed operator
			// value not empty, add the last identifier or literal to the token list
			if (!value.empty()) {
				// add to the token list
				if (only_digits) {
					tokens.push_back(
						std::make_shared<NumberLiteral>(
							atoi(value.c_str()),
							position-value.size(),
							value.size()
						)
					);
				} else {
					if (start_with_digits) {
						return ParseResult(2, position-value.size());
					}
					tokens.push_back(std::make_shared<Variable>(
						value, position-value.size(), value.size()
					));
				}
				// clear the value
				value = "";
			}
			// add operator
			tokens.push_back(std::make_shared<Operator>(c, position));
			only_digits = true;
			start_with_digits = false;
		} else if (c == '_') {
			if (value.empty()) {
				return ParseResult(3, position);
			}
			value += c;
		} else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
			// appdend the letter or '_'
			value += c;
			only_digits = false;
		} else if (c >= '0' && c <= '9') {
			if (value.empty()) start_with_digits = true;
			value += c;
		} else {
			return ParseResult(1, position);
		}
	}

	size_t last_space = 0;
	for (size_t i = expr.length()-1; i > 0; --i) {
		if (expr[i] == ' ') ++last_space;
		else break;
	}

	// the last token
	if (!value.empty()) {
		if (only_digits) {
			tokens.push_back(std::make_shared<NumberLiteral>(
				atoi(value.c_str()), expr.size()-last_space-value.size(), value.size()
			));
		} else {
			if (start_with_digits) {
				return ParseResult(2, expr.size()-last_space-value.size());
			}
			tokens.push_back(std::make_shared<Variable>(
				value, expr.size()-last_space-value.size(), value.size()
			));
		}
	}

	return ParseResult(0);
}

}			// namespace ecl

