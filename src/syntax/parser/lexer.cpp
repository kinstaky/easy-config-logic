/*
 * This file is part of the context-free grammar library.
 */

#include "syntax/parser/lexer.h"

#include <memory>
#include <string>
#include <vector>

#include "syntax/parser/token.h"

namespace ecl {

int Lexer::Analyse(const std::string &expr, std::vector<TokenPtr> &tokens) {
	// value
	std::string value = "";
	// only digits
	bool only_digits = true;
	// identifier starts with digits
	bool start_with_digits = false;

	for (char c : expr) {

		// ignore the blank character
		if (c == ' ') continue;

		if (c == '(' || c == ')' || c == '&' || c == '|' || c == '=' || c == '/') {
			// the needed operator
			// value not empty, add the last identifier or literal to the token list
			if (!value.empty()) {
				// add to the token list
				if (only_digits) {
					tokens.push_back(
						std::make_shared<NumberLiteral>(atoi(value.c_str()))
					);
				} else {
					if (start_with_digits) return -2;
					tokens.push_back(std::make_shared<Variable>(value));
				}
				// clear the value
				value = "";
			}
			// add operator
			tokens.push_back(std::make_shared<Operator>(c));
			only_digits = true;
			start_with_digits = false;
		} else if (c == '_') {
			if (value.empty()) return -3;
			value += c;
		} else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
			// appdend the letter or '_'
			value += c;				
			only_digits = false;
		} else if (c >= '0' && c <= '9') {
			if (value.empty()) start_with_digits = true;
			value += c;
		} else {
			return -1;
		}
	}

	if (!value.empty()) {
		if (only_digits) {
			tokens.push_back(std::make_shared<NumberLiteral>(atoi(value.c_str())));
		} else {
			if (start_with_digits) return -2;
			tokens.push_back(std::make_shared<Variable>(value));
		}
	}

	return 0;
}

}			// namespace ecl

