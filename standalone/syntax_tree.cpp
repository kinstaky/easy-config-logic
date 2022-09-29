/*
 * This tool shows the syntax tree generated by lexer and syntax parser.
 */

#include <iostream>
#include <string>
#include <vector>

#include "syntax/parser/lexer.h"
#include "syntax/parser/syntax_parser.h"
#include "syntax/logical_grammar.h"

int main() {
	std::string line;
	std::getline(std::cin, line);

	// lexer analyse
	ecl::Lexer lex;
	std::vector<ecl::TokenPtr> tokens;
	lex.Analyse(line, tokens);

	// syntax parser parse
	ecl::LogicalGrammar grammar;
	ecl::SLRSyntaxParser<bool> parser(&grammar);
	parser.Parse(tokens);

	parser.PrintTree(parser.Root());

	return 0;
}