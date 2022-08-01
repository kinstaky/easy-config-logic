#include "syntax/parser/syntax_parser.h"

#include <gtest/gtest.h>

#include "syntax/parser/token.h"
#include "syntax/logical_grammar.h"
#include "syntax/arithmetic_grammar.h"

using namespace ecc;


// check action table
// add multi grammar
const std::vector<std::vector<int>> add_multi_action_type = {
	{-1, 2, -1, 2, -1, 2, 1, -1, 1, -1},			// I0
	{-1, -1, 1, -1, -1, -1, -1, -1, -1, 0},			// I1
	{-1, -1, 3, -1, 1, -1, -1, 3, -1, 3},			// I2
	{-1, -1, 3, -1, 3, -1, -1, 3, -1, 3},			// I3
	{-1, 2, -1, 2, -1, 2, 1, -1, 1, -1},			// I4
	{-1, -1, 3, -1, 3, -1, -1, 3, -1, 3},			// I5
	{-1, -1, -1, 2, -1, 2, 1, -1, 1, -1},			// I6
	{-1, -1, -1, -1, -1, 2, 1, -1, 1, -1},			// I7
	{-1, -1, 1, -1, -1, -1, -1, 1, -1, -1},		// I8
	{-1, -1, 3, -1, 1, -1, -1, 3, -1, 3},			// I9
	{-1, -1, 3, -1, 3, -1, -1, 3, -1, 3},			// I10
	{-1, -1, 3, -1, 3, -1, -1, 3, -1, 3}			// I11
};
const std::vector<std::vector<int>> add_multi_action_value = {
	{0, 1, 0, 2, 0, 3, 4, 0, 5, 0},					// I0
	{0, 0, 6, 0, 0, 0, 0, 0, 0, 0},					// I1
	{0, 0, 2, 0, 7, 0, 0, 2, 0, 2},					// I2
	{0, 0, 4, 0, 4, 0, 0, 4, 0, 4},					// I3
	{0, 8, 0, 2, 0, 3, 4, 0, 5, 0},					// I4
	{0, 0, 6, 0, 6, 0, 0, 6, 0, 6},					// I5
	{0, 0, 0, 9, 0, 3, 4, 0, 5, 0},					// I6
	{0, 0, 0, 0, 0, 10, 4, 0, 5, 0},				// I7
	{0, 0, 6, 0, 0, 0, 0, 11, 0, 0},				// I8
	{0, 0, 1, 0, 7, 0, 0, 1, 0, 1},					// I9
	{0, 0, 3, 0, 3, 0, 0, 3, 0, 3},					// I10
	{0, 0, 5, 0, 5, 0, 0, 5, 0, 5}					// I11
};
// arithmetic grammar
const std::vector<std::vector<int>> arithmetic_action_type = {};
const std::vector<std::vector<int>> arithmetic_action_value ={};
// logical grammar
const std::vector<std::vector<int>> logical_action_type = {
	{-1, 2, -1, 2, -1, 1, -1, 1, -1},				// I0
	{-1, -1, 1, -1, 1, -1, -1, -1, 0},				// I1
	{-1, -1, 3, -1, 3, -1, 3, -1, 3},				// I2
	{-1, 2, -1, 2, -1, 1, -1, 1, -1},				// I3
	{-1, -1, 3, -1, 3, -1, 3, -1, 3},				// I4
	{-1, -1, -1, 2, -1, 1, -1, 1, -1},				// I5
	{-1, -1, -1, 2, -1, 1, -1, 1, -1},				// I6
	{-1, -1, 1, -1, 1, -1, 1, -1, -1},				// I7
	{-1, -1, 3, -1, 3, -1, 3, -1, 3},				// I8
	{-1, -1, 3, -1, 3, -1, 3, -1, 3},				// I9
	{-1, -1, 3, -1, 3, -1, 3, -1, 3}				// I10
};
const std::vector<std::vector<int>> logical_action_value = {
	{0, 1, 0, 2, 0, 3, 0, 4, 0},					// I0
	{0, 0, 5, 0, 6, 0, 0, 0, 0},					// I1
	{0, 0, 3, 0, 3, 0, 3, 0, 3},					// I2
	{0, 7, 0, 2, 0, 3, 0, 4, 0},					// I3
	{0, 0, 5, 0, 5, 0, 5, 0, 5},					// I4
	{0, 0, 0, 8, 0, 3, 0, 4, 0},					// I5
	{0, 0, 0, 9, 0, 3, 0, 4, 0},					// I6
	{0, 0, 5, 0, 6, 0, 10, 0, 0},					// I7
	{0, 0, 1, 0, 1, 0, 1, 0, 1},					// I8
	{0, 0, 2, 0, 2, 0, 2, 0, 2},					// I9
	{0, 0, 4, 0, 4, 0, 4, 0, 4}						// I10
};




// check attach and evaluate
// add multi grammar
// add multi input
const std::vector<std::vector<TokenPtr>> kAddMultiTokens = {
	{
		std::make_shared<Identifier>("A"), std::make_shared<Operator>("+"),
		std::make_shared<Identifier>("A")
	},
	{
		std::make_shared<Identifier>("B"), std::make_shared<Operator>("*"),
		std::make_shared<Identifier>("B")
	},
	{
		std::make_shared<Identifier>("a"), std::make_shared<Operator>("+"), 
		std::make_shared<Identifier>("b"), std::make_shared<Operator>("*"),
		std::make_shared<Identifier>("c")	
	},
	{
		std::make_shared<Operator>("("), std::make_shared<Identifier>("A"),
		std::make_shared<Operator>("+"), std::make_shared<Identifier>("B"),
		std::make_shared<Operator>(")"), std::make_shared<Operator>("*"),
		std::make_shared<Identifier>("C")
	}
};
// add multi identifier values
const std::vector<std::vector<double>> kAddMultiValue = {
	{25.0},
	{32.0},
	{6.0, 3.0, 7.0},
	{6.0, 3.0, 7.0}
};
// add multi output
const std::vector<double> kAddMultiResult = {
	50.0,
	1024.0,
	27.0,
	63.0
};
// logical grammar
// logical input
const std::vector<std::vector<TokenPtr>> kLogicalTokens = {
	{
		std::make_shared<Identifier>("A"), std::make_shared<Operator>("|"),
		std::make_shared<Identifier>("A")
	},
	{
		std::make_shared<Identifier>("A"), std::make_shared<Operator>("|"),
		std::make_shared<Identifier>("A")
	},
	{
		std::make_shared<Identifier>("B"), std::make_shared<Operator>("&"),
		std::make_shared<Identifier>("B")
	},
	{
		std::make_shared<Identifier>("B"), std::make_shared<Operator>("&"),
		std::make_shared<Identifier>("B")
	},
	{
		std::make_shared<Identifier>("A"), std::make_shared<Operator>("|"),
		std::make_shared<Operator>("("), std::make_shared<Identifier>("B"),
		std::make_shared<Operator>("&"), std::make_shared<Identifier>("C"),
		std::make_shared<Operator>(")")
	},
	{
		std::make_shared<Identifier>("A"), std::make_shared<Operator>("|"),
		std::make_shared<Operator>("("), std::make_shared<Identifier>("B"),
		std::make_shared<Operator>("&"), std::make_shared<Identifier>("C"),
		std::make_shared<Operator>(")")
	},
	{
		std::make_shared<Operator>("("), std::make_shared<Identifier>("A"),
		std::make_shared<Operator>("|"), std::make_shared<Identifier>("B"),
		std::make_shared<Operator>(")"), std::make_shared<Operator>("&"),
		std::make_shared<Identifier>("C")
	},
	{
		std::make_shared<Operator>("("), std::make_shared<Identifier>("A"),
		std::make_shared<Operator>("|"), std::make_shared<Identifier>("B"),
		std::make_shared<Operator>(")"), std::make_shared<Operator>("&"),
		std::make_shared<Identifier>("C")
	}
};
// logical identifier values
const std::vector<std::vector<int>> kLogicalValue = {
	{true},
	{false},
	{true},
	{false},
	{true, false, false},
	{true, true, false},
	{true, false, false},
	{true, true, false}
};
// logical output
const std::vector<bool> kLogicalResult = {
	true,
	false,
	true,
	false,
	true,
	true,
	false,
	false
};




// TEST(SLRSyntaxParserTest, ActionTable) {

// }



// TEST(SLRSyntaxParserTest, HandleParseError) {

// }



TEST(SLRSyntaxParserTest, ActionTable) {
	// check add multi grammar
	{
		std::cout << "==check add multi grammar==" << std::endl;
		const std::vector<std::vector<int>> &action_type = add_multi_action_type;
		const std::vector<std::vector<int>> &action_value = add_multi_action_value;
		AddMultiGrammar grammar;
		SLRSyntaxParser parser(&grammar);
		ActionTable *table = parser.GetActionTable();
		for (size_t c = 0; c < action_type.size(); ++c) {
			for (size_t s = 0; s < action_type[0].size(); ++s) {
				Action *action = table->GetAction(c, s);
				EXPECT_EQ(action->type, action_type[c][s])
					<< "error action type collection " << c
					<< ", symbol " << s;
				if (action->type == Action::kTypeShift || action->type == Action::kTypeGoto) {
					EXPECT_EQ(action->collection, action_value[c][s])
						<< "error action value collection, collection " << c
						<< ", symbol " << s;
				} else if (action->type == Action::kTypeReduce) {
					int offset = 0;
					for (int i = 0; i < grammar.ProductionSetSize(); ++i) {
						bool found = false;
						ProductionFactorySet<double> *set = grammar.ProductionSet(i);
						for (size_t j = 0; j < set->size(); ++j) {
							if ((*set)[j] == action->production) {
								offset += j;
								found = true;
							}
						}
						if (found) break;
						offset += set->size();
					}
					EXPECT_EQ(offset, action_value[c][s])
						<< "error action value production, collection " << c
						<< ", symbol " << s;
				}
			}
		}
	}



	// check logical grammar
	{
		std::cout << "==check logical grammar==" << std::endl;
		const std::vector<std::vector<int>> &action_type = logical_action_type;
		const std::vector<std::vector<int>> &action_value = logical_action_value;
		LogicalGrammar grammar;
		SLRSyntaxParser parser(&grammar);
		ActionTable *table = parser.GetActionTable();
		for (size_t c = 0; c < action_type.size(); ++c) {
			for (size_t s = 0; s < action_type[0].size(); ++s) {
				Action *action = table->GetAction(c, s);
				EXPECT_EQ(action->type, action_type[c][s])
					<< "error action type collection " << c
					<< ", symbol " << s;
				if (action->type == Action::kTypeShift || action->type == Action::kTypeGoto) {
					EXPECT_EQ(action->collection, action_value[c][s])
						<< "error action value collection, collection " << c
						<< ", symbol " << s;
				} else if (action->type == Action::kTypeReduce) {
					int offset = 0;
					for (int i = 0; i < grammar.ProductionSetSize(); ++i) {
						bool found = false;
						ProductionFactorySet<bool> *set = grammar.ProductionSet(i);
						for (size_t j = 0; j < set->size(); ++j) {
							if ((*set)[j] == action->production) {
								offset += j;
								found = true;
							}
						}
						if (found) break;
						offset += set->size();
					}
					EXPECT_EQ(offset, action_value[c][s])
						<< "error action value production, collection " << c
						<< ", symbol " << s;
				}
			}
		}
	}
}



TEST(SLRSyntaxParserTest, Parse) {
	// check add multi grammar
	{
		int index = 0;
		const std::vector<std::vector<TokenPtr>> &tokens_list = kAddMultiTokens;
		const std::vector<std::vector<double>> &value = kAddMultiValue;
		const std::vector<double> &result = kAddMultiResult;
		for (auto tokens : tokens_list) {
			AddMultiGrammar grammar;
			SLRSyntaxParser parser(&grammar);
			ASSERT_EQ(parser.Parse(tokens), 0);

			// parser.PrintTree(parser.Root());
			for (size_t i = 0; i < value[index].size(); ++i) {
				EXPECT_EQ(parser.AttachIdentifier(i, (void*)&(value[index][i])), 0);
			}
			EXPECT_EQ(result[index], parser.Eval())
				<< "error evaluated value, index " << index;
			
			std::cout << "====Finish test case " << index << std::endl;
			++index;
		}
	}
	// check logical grammar
	{
		int index = 0;
		const std::vector<std::vector<TokenPtr>> &tokens_list = kLogicalTokens;
		const std::vector<std::vector<int>> &value = kLogicalValue;
		const std::vector<bool> &result = kLogicalResult;
		for (auto tokens : tokens_list) {
			LogicalGrammar grammar;
			SLRSyntaxParser parser(&grammar);
			ASSERT_EQ(parser.Parse(tokens), 0);

			// parser.PrintTree(parser.Root());
			for (size_t i = 0; i < value[index].size(); ++i) {
				EXPECT_EQ(parser.AttachIdentifier(i, (void*)&(value[index][i])), 0);
			}
			EXPECT_EQ(result[index], parser.Eval())
				<< "error evaluated value, index " << index;
			
			std::cout << "====Finish test case " << index << std::endl;
			++index;
		}
	}
}