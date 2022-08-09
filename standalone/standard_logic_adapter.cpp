#include <iostream>
#include <string>
#include <vector>



#include "standardize/standard_logic_node.h"
#include "standardize/standard_logic_tree.h"
#include "syntax/parser/lexer.h"
#include "syntax/parser/syntax_parser.h"
#include "syntax/logical_grammar.h"


int main() {
	std::string line;
	std::getline(std::cin, line);

	ecc::Lexer lexer;
	ecc::LogicalGrammar grammar;
	ecc::SLRSyntaxParser<bool> parser(&grammar);
	std::vector<ecc::TokenPtr> tokens;


	lexer.Analyse(line, tokens);
	parser.Parse(tokens);

	ecc::StandardLogicTree tree(parser.Root());

	std::cout << tree << std::endl;
}