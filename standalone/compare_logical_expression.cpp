/*
 * This tool compare the two logic expressions and 
 * recognize whether they are equal.
 */

#include <iostream>
#include <string>
#include <vector>


#include "syntax/parser/lexer.h"
#include "syntax/parser/syntax_parser.h"
#include "syntax/logical_grammar.h"
#include "syntax/logic_comparer.h"



int main() {
	std::string line[2];
	for (int i = 0; i != 2; ++i) {
		std::getline(std::cin, line[i]);
	}

	ecl::LogicComparer comparer;
	if (comparer.Compare(line[0], line[1])) {
		std::cout << "\033[0;32m" << "YES" << "\033[0m" << std::endl;
	} else {
		std::cout << "\033[1;31m" << "NO" << "\033[0m" << std::endl;
	}
	

	return 0;
}