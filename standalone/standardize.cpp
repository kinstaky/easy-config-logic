#include <iostream>
#include <string>
#include <vector>

#include "syntax/parser/lexer.h"
#include "syntax/logic_downscale_grammar.h"
#include "syntax/parser/syntax_parser.h"
#include "standardize/standard_logic_downscale_tree.h"

int main() {
	std::string line;
	std::getline(std::cin, line);

	// lexer
	ecl::Lexer lexer;
	// tokens
	std::vector<ecl::TokenPtr> tokens;
	// lexer parse and get tokens
	lexer.Analyse(line, tokens);

	// grammar
	ecl::LogicDownscaleGrammar grammar;
	// syntax parser
	ecl::SLRSyntaxParser<int> parser(&grammar);

	if (tokens.size() < 3 || tokens[1]->Name() != "=") {
		std::vector<ecl::TokenPtr> full_tokens;
		full_tokens.push_back(std::make_shared<ecl::Variable>("_Left"));
		full_tokens.push_back(std::make_shared<ecl::Operator>("="));
		for (size_t i = 0; i < tokens.size(); ++i) full_tokens.push_back(tokens[i]);
		if (parser.Parse(full_tokens)) {
			std::cerr << "Error: Parse failed.\n";
			return -1;
		}
	} else {
		if (parser.Parse(tokens)) {
			std::cerr << "Error: Parse failed.\n";
			return -1;
		}
	}
	ecl::StandardLogicDownscaleTree tree((ecl::Production<int>*)(parser.Root()->Child(2)));
	std::cout << tree << std::endl;
	return 0;
}