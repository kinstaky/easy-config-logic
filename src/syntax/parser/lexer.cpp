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
	std::string value = "";
	for (char c : expr) {

		// ignore the blank character
		if (c == ' ')
			continue;


		if (c == '(' || c == ')' || c == '&' || c == '|') {				// the needed operator

			// value not empty, add the identifier to the token list
			if (!value.empty()) {
				tokens.emplace_back(std::make_shared<Identifier>(value));		// add to the token list
				value = "";							// clear the value
			}
			tokens.emplace_back(std::make_shared<Operator>(c));

		} else if (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {

			value += c;				// appdend the letter or '_'

		} else if (c >= '0' && c <= '9') {

			if (value.empty())
				return -1;

			value += c;

		} else {

			return -1;

		}
	}

	if (!value.empty()) {
		tokens.emplace_back(std::make_shared<Identifier>(value));
	}

	return 0;
}

}			// namespace ecl

