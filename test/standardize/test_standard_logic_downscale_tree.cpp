#include "standardize/standard_logic_downscale_tree.h"

#include <vector>
#include <string>
#include <sstream>

#include <gtest/gtest.h>

#include "syntax/parser/lexer.h"
#include "syntax/logic_downscale_grammar.h"
#include "syntax/parser/syntax_parser.h"
#include "syntax/logic_comparer.h"

using namespace ecl;

const std::vector<std::string> kExpression = {
	"A|B",
	"(A&B)|C",
	"A / 5",
	"((A&B)|C)/4",
	"((A&B)|C)/10 | B/5",
	"(((A&B)|C)/10 & D/10 & E) | ((F&C)|D)/3 | G/5",
};

const std::vector<int> kLayer = {
	0,
	0,
	1,
	1,
	1,
	1,
};

const std::vector<std::string> kOutput = {
	"A | B",
	"(A | C) & (B | C)",
	"A / 5",
	"((A | C) & (B | C)) / 4",
	"(((A | C) & (B | C)) / 10) | (B / 5)",
	"((((A | C) & (B | C)) / 10) | (((C | D) & (D | F)) / 3) | (G / 5))"\
	" & ((D / 10) | (((C | D) & (D | F)) / 3) | (G / 5))"\
	" & (E | (((C | D) & (D | F)) / 3) | (G / 5))",
};


TEST(StandardLogicDownscaleTreeTest, Output) {
	for (size_t i = 0; i < kOutput.size(); ++i) {
		Lexer lexer;
		LogicDownscaleGrammar grammar;
		SLRSyntaxParser parser(&grammar);
		std::vector<TokenPtr> tokens;

		ASSERT_EQ(lexer.Analyse("LEFT="+kExpression[i], tokens), 0)
			<< "Error: Lexer analyse " << i;
		ASSERT_EQ(parser.Parse(tokens), 0)
			<< "Error: Parser parse " << i;

		ASSERT_EQ(parser.Root()->Eval(), kLayer[i])
			<< "Error: Evaluate layers " << i;

		StandardLogicDownscaleTree tree(
			(Production<int>*)(parser.Root()->Child(2))
		);

		std::stringstream ss;
		ss << tree;
		EXPECT_EQ(ss.str(), kOutput[i])
			<< "Error: Ouptput string " << i;
	}
}