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
	// with number literal 0 or 1
	"0|1",
	"A&0",
	"(A|0)&B",
	"(A&B&0)|(C&0)",
	"(A&B&1)|C",
	"(A | B | 1) / 5",
	"(A & 0 & (B | C)) / 4",
	"((A|B|1)/5) & C",
	"(((A&B)/10) & ((A|B|1)/5)) | C | D"
};

const std::vector<int> kLayer = {
	0,
	0,
	1,
	1,
	1,
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	1,
	1,
	1
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
	"1",
	"0",
	"A & B",
	"0",
	"(A | C) & (B | C)",
	"1",
	"0",
	"C",
	"((A & B) / 10) | C | D"
};


TEST(StandardLogicDownscaleTreeTest, Output) {
	for (size_t i = 0; i < kOutput.size(); ++i) {
		Lexer lexer;
		LogicDownscaleGrammar grammar;
		SLRSyntaxParser<int> parser(&grammar);
		std::vector<TokenPtr> tokens;

		ASSERT_TRUE(lexer.Analyse("LEFT="+kExpression[i], tokens).Ok())
			<< "Error: Lexer analyse " << i;
		ASSERT_TRUE(parser.Parse(tokens).Ok())
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