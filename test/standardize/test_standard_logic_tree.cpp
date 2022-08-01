#include "standardize/standard_logic_tree.h"

#include <vector>
#include <string>
#include <chrono>
#include <sstream>

#include <gtest/gtest.h>

#include "syntax/parser/lexer.h"
#include "syntax/logical_grammar.h"
#include "syntax/parser/syntax_parser.h"
#include "syntax/logic_comparer.h"

using namespace ecc;


const std::vector<std::string> kExpression = {
	"A",
	"A|B",
	"A & B | C & D",
	"(A&B) | (C&D)",
	"((A&B) | (C&D)) & (E|F)",
	"((A|B) & (C|D)) | (E&F)",
	"(A & B & C) | (A & B & D) | (A & C & E) | (A & F)",
	"(A & B & C) | (A & B & D) | (A & C & E) | (A & F) | B | G",
	"((A&B&C)|D|(E&F)|H|I)&(J|K)",
	"(A0&A3) & ( (A4|A7|A8|A12|A13|A15) | (B0|B3|B4|B7|B8|B11) | (C0|C3) | (((C7&C11)|C4|C8) & C12) )",
	"(((A&B&C)|D|(E&F)) & ((G&H)|(I&J))) | (K&L) | (((M&N&O)|(P&Q)|(R&S&T)) & (U|V) & W) | (X&Y) | Z"
};
const std::vector<std::string> kOutput = {
	"A",
	"A | B",
	"(A | C) & (B | C) & D",
	"(A | C) & (B | C) & (A | D) & (B | D)",
	"(E | F) & (A | C) & (B | C) & (A | D) & (B | D)",
	"(A | B | E) & (A | B | F) & (C | D | E) & (C | D | F)",
	"(C | D | F) & (B | C | F) & (B | E | F) & A",
	"(A | B | G) & (B | C | D | G) & (B | E | F | G)",
	"(J | K) & (A | D | E | H | I) & (B | D | E | H | I) & (C | D | E | H | I) & (A | D | F | H | I) & (B | D | F | H | I) & (C | D | F | H | I)"
};



// TEST(StandardLogicTreeTest, CompareValue) {
// 	for (size_t i = 0; i < kExpression.size(); ++i) {
// 		Lexer lexer;
// 		LogicalGrammar grammar;
// 		SLRSyntaxParser parser(&grammar);
// 		std::vector<TokenPtr> tokens;

// 		ASSERT_EQ(lexer.Analyse(kExpression[i], tokens), 0)
// 			<< "Error: lexer analyse " << i;
// 		ASSERT_EQ(parser.Parse(tokens), 0)
// 			<< "Error: parser parse " << i;
		

// 		StandardLogicTree tree(parser.Root());

// 		EXPECT_LE(tree.Root()->Depth(), 2)
// 			<< "Error: depth larger than 2 " << i;

// 		std::stringstream ss;
// 		ss << tree;

// 		LogicComparer comparer;
// 		EXPECT_TRUE(comparer.Compare(kExpression[i], ss.str()))
// 			<< "Error: compare not equal " << i;

// 		std::cout << "finish test case " << i << std::endl;
// 		std::cout << "-----------------------------" << std::endl;
// 	}
// }




// TEST(StandardLogicTreeTest, OutputString) {
// 	for (size_t i = 0; i < kOutput.size(); ++i) {
// 		Lexer lexer;
// 		LogicalGrammar grammar;
// 		SLRSyntaxParser parser(&grammar);
// 		std::vector<TokenPtr> tokens;

// 		ASSERT_EQ(lexer.Analyse(kExpression[i], tokens), 0)
// 			<< "Error: lexer analyse " << i;
// 		ASSERT_EQ(parser.Parse(tokens), 0)
// 			<< "Error: parser parse " << i;
		

// 		StandardLogicTree tree(parser.Root());

// 		// tree.PrintTree();
// 		std::stringstream ss;
// 		ss << tree;
// 		EXPECT_EQ(ss.str(), kOutput[i])
// 			<< "Error ouptput string " << i;
// 		std::cout << ss.str() << std::endl;

// 		std::cout << "finish test case " << i << std::endl;
// 		std::cout << "-----------------------------" << std::endl;
// 	}
// }




TEST(StandardLogicTreeTest, Time) {
	std::vector<double> time_cost;
	for (size_t i = 0; i < kExpression.size(); ++i) {
		Lexer lexer;
		LogicalGrammar grammar;
		SLRSyntaxParser parser(&grammar);
		std::vector<TokenPtr> tokens;
		lexer.Analyse(kExpression[i], tokens);
		parser.Parse(tokens);
		

		auto start = std::chrono::high_resolution_clock::now();
		auto stop = start;

		StandardLogicTree tree(parser.Root());

		stop = std::chrono::high_resolution_clock::now();

		std::cout << tree << std::endl;

		time_cost.push_back(std::chrono::duration_cast<std::chrono::microseconds>(stop-start).count());
		std::cout << "case " << i << " first layer branch size " << tree.Root()->BranchSize() << std::endl;
	}
	for (size_t i = 0; i < time_cost.size(); ++i) {
		std::cout << "test case " << i << " cost time " << time_cost[i] << " us" << std::endl;
	}
}