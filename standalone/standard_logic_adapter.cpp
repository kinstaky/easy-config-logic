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

	ecl::Lexer lexer;
	ecl::LogicalGrammar grammar;
	ecl::SLRSyntaxParser<bool> parser(&grammar);
	std::vector<ecl::TokenPtr> tokens;


	lexer.Analyse(line, tokens);
	parser.Parse(tokens);

	ecl::StandardLogicTree tree(parser.Root());

	std::cout << tree << std::endl;
}