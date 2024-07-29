#include "syntax/parser/grammar.h"
#include "syntax/arithmetic_grammar.h"
#include "syntax/logical_grammar.h"

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "syntax/parser/token.h"

using namespace ecl;

// first list
// add multi grammar
const std::vector<std::vector<std::string>> add_multi_grammar_first_list = {
	{"(", ""},					// s
	{"(", ""},					// e
	{"(", ""},					// t
	{"(", ""}					// f
};
const std::vector<std::vector<int>> add_multi_grammar_first_type = {
	{kSymbolType_Operator, kSymbolType_Variable},		// s
	{kSymbolType_Operator, kSymbolType_Variable},		// e
	{kSymbolType_Operator, kSymbolType_Variable},		// t
	{kSymbolType_Operator, kSymbolType_Variable}		// f
};
const std::vector<bool> add_multi_grammar_frist_empty = {
	false,					// s
	false,					// e
	false,					// t
	false					// f
};


// arithmetic grammar
const std::vector<std::vector<std::string>> arithmetic_grammar_first_list = {
	{"(", ""},					// s
	{"(", ""},					// e
	{"(", ""},					// t
	{"(", ""}					// f
};
const std::vector<std::vector<int>> arithmetic_grammar_first_type = {
	{kSymbolType_Operator, kSymbolType_Variable},		// s
	{kSymbolType_Operator, kSymbolType_Variable},		// e
	{kSymbolType_Operator, kSymbolType_Variable},		// t
	{kSymbolType_Operator, kSymbolType_Variable}		// f
};
const std::vector<bool> arithmetic_grammar_frist_empty = {
	false,					// s
	false,					// e
	false,					// t
	false					// f
};


// logical grammar
const std::vector<std::vector<std::string>> logical_grammar_first_list = {
	{"(", ""},					// s
	{"(", ""},					// e
	{"(", ""}					// t
};
const std::vector<std::vector<int>> logical_grammar_first_type = {
	{kSymbolType_Operator, kSymbolType_Variable},		// s
	{kSymbolType_Operator, kSymbolType_Variable},		// e
	{kSymbolType_Operator, kSymbolType_Variable}		// t
};
const std::vector<bool> logical_grammar_frist_empty = {
	false,					// s
	false,					// e
	false					// t
};




// following list
// add multi grammar
const std::vector<std::vector<std::string>> add_multi_following_list = {
	{"$"},							// s
	{"+", ")", "$"},				// e
	{"*", "+", ")", "$"},			// t
	{"*", "+", ")", "$"}			// f
};
const std::vector<std::vector<int>> add_multi_following_type = {
	{kSymbolType_Operator},																		// s
	{kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator},							// e
	{kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator},	// t
	{kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator}	// f
};
// arithmetic grammar
const std::vector<std::vector<std::string>> arithmetic_following_list = {
	{"$"},								// s
	{"+", "-", ")", "$"},				// e
	{"+", "-", ")", "*", "/", "$"},		// t
	{"+", "-", ")", "*", "/", "$"} 	    // f
};
const std::vector<std::vector<int>> arithmetic_following_type = {
	{kSymbolType_Operator},
	{kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator},
	{kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator},
	{kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator}
};

// logical grammar
const std::vector<std::vector<std::string>> logical_following_list = {
	{"$"},								// s
	{"|", "&", ")", "$"},				// e
	{"|", "&", ")", "$"}				// t
};
const std::vector<std::vector<int>> logical_following_type = {
	{kSymbolType_Operator},
	{kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator},
	{kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator, kSymbolType_Operator}
};




// goto table
// add multi grammar
const std::vector<std::vector<int>> add_multi_goto_table = {
	{-1, 1, -1, 2, -1, 3, 4, -1, 5, -1},			// I0
	{-1, -1, 6, -1, -1, -1, -1, -1, -1, -1},		// I1
	{-1, -1, -1, -1, 7, -1, -1, -1, -1, -1},		// I2
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},		// I3
	{-1, 8, -1, 2, -1, 3, 4, -1, 5, -1},			// I4
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},		// I5
	{-1, -1, -1, 9, -1, 3, 4, -1, 5, -1},			// I6
	{-1, -1, -1, -1, -1, 10, 4, -1, 5, -1},			// I7
	{-1, -1, 6, -1, -1, -1, -1, 11, -1, -1},		// I8
	{-1, -1, -1, -1, 7, -1, -1, -1, -1, -1},		// I9
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},		// I10
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}		// I11
};
// arithmetci grammar
const std::vector<std::vector<int>> arithmetic_goto_table = {};
// logical grammar
const std::vector<std::vector<int>> logical_goto_table = {
	{-1, 1, -1, 2, -1, 3, -1, 4, -1},				// I0
	{-1, -1, 5, -1, 6, -1, -1, -1, -1},				// I1
	{-1, -1, -1, -1, -1, -1, -1, -1, -1},			// I2
	{-1, 7, -1, 2, -1, 3, -1, 4, -1},				// I3
	{-1, -1, -1, -1, -1, -1, -1, -1, -1},			// I4
	{-1, -1, -1, 8, -1, 3, -1, 4, -1},				// I5
	{-1, -1, -1, 9, -1, 3, -1, 4, -1},				// I6
	{-1, -1, 5, -1, 6, -1, 10, -1, -1},				// I7
	{-1, -1, -1, -1, -1, -1, -1, -1, -1},			// I8
	{-1, -1, -1, -1, -1, -1, -1, -1, -1},			// I9
	{-1, -1, -1, -1, -1, -1, -1, -1, -1}			// I10
};






bool SymbolInFirst(
	Symbol *symbol, 
	const std::vector<std::string> &first, 
	const std::vector<int> &type
) {

	if (symbol->Type() == kSymbolType_Variable) {

		/// check whether this symbol is an identifier
		for (size_t i = 0; i < type.size(); ++i) {
			if (type[i] == kSymbolType_Variable) {
				return true;
			}
		}
	} else {

		// check symbols except identifier
		for (size_t i = 0; i < type.size(); ++i) {
			if (type[i] == symbol->Type() && first[i] == ((Token*)symbol)->Name()) {
				return true;
			}
		}
	}

	return false;
}



bool SymbolInFollow(
	size_t index, 
	const std::vector<std::string> &follow,
	const std::vector<int> &type, 
	const std::vector<Symbol*> &symbols
) {

	if (index == symbols.size()) {
		// end symbol
		for (auto s : follow) {
			if (s == "$") {
				return true;
			}
		}
		return false;
	} else {
		// other terminal symbols
		Symbol *symbol = symbols[index];
		for (size_t i = 0; i < follow.size(); ++i) {
			if (symbol->Type() != type[i]) {
				continue;
			}
			switch (symbol->Type()) {
				case kSymbolType_Variable:
					return true;
				case kSymbolType_Operator:
					if (((Operator*)symbol)->Name() == follow[i]) {
						return true;
					}
					break;
				default:
					;
			}	
		}
	}
	return false;
}




TEST(GrammarTest, NotLanguage) {
	Grammar<bool> grammar;

	// terminal symbols
	Symbol *identifier = new Symbol(kSymbolType_Variable);
	Operator *op_not = new Operator("~");

	// non-terminal symbols
	// production sets
	ProductionFactorySet<bool> *production_set_s = new ProductionFactorySet<bool>;
	ProductionFactorySet<bool> *production_set_e = new ProductionFactorySet<bool>;

	// production factories
	ProductionFactory<bool> *production_s_e = new ProductionFactory<bool>(
		production_set_s,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<bool>(symbols[0]);
		}
	);
	ProductionFactory<bool> *production_e_not_e = new ProductionFactory<bool>(
		production_set_e,
		2,
		[](const std::vector<Symbol*> &symbols) {
			return !Evaluate<bool>(symbols[1]);
		}
	);
	ProductionFactory<bool> *production_e_id = new ProductionFactory<bool>(
		production_set_e,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<bool>(symbols[0]);
		}
	);
	
	// set children
	production_s_e->SetChildren(production_set_e);
	production_e_not_e->SetChildren(op_not, production_set_e);
	production_e_id->SetChildren(identifier);

	// // add production factories to sets
	production_set_s->AddProductionFactory(production_s_e);
	production_set_e->AddProductionFactory(production_e_not_e);
	production_set_e->AddProductionFactory(production_e_id);

	// add production factory sets to grammar
	grammar.AddProductionSet(production_set_s, true);
	grammar.AddProductionSet(production_set_e, false);


	// set children
	EXPECT_EQ(production_set_s->size(), 1);
	EXPECT_EQ(production_set_e->size(), 2);
	EXPECT_EQ((*production_set_s)[0]->Child(0), production_set_e);
	EXPECT_EQ((*production_set_e)[0]->Child(0), op_not);
	EXPECT_EQ((*production_set_e)[0]->Child(1), production_set_e);
	EXPECT_EQ((*production_set_e)[1]->Child(0), identifier);
	EXPECT_EQ((*production_set_e)[0]->Child(0)->Type(), kSymbolType_Operator);
	EXPECT_EQ((*production_set_e)[0]->Child(1)->Type(), kSymbolType_ProductionFactorySet);
	EXPECT_EQ((*production_set_e)[1]->Child(0)->Type(), kSymbolType_Variable);


	// symbol list
	std::vector<Symbol*> symbol_list = grammar.SymbolList();
	EXPECT_EQ(symbol_list.size(), 4);
	EXPECT_EQ(symbol_list[0], production_set_s);
	EXPECT_EQ(symbol_list[1], production_set_e);
	EXPECT_EQ(symbol_list[2], op_not);
	EXPECT_EQ(symbol_list[3], identifier);


	// first list
	std::vector<int> first_list = grammar.First(0);
	EXPECT_EQ(first_list.size(), 2);
	EXPECT_EQ(first_list[0], 2);
	EXPECT_EQ(first_list[1], 3);
	first_list = grammar.First(1);
	EXPECT_EQ(first_list.size(), 2);
	EXPECT_EQ(first_list[0], 2);
	EXPECT_EQ(first_list[1], 3);


	// following list
	std::vector<int> following_list = grammar.Following(0);
	EXPECT_EQ(following_list.size(), 1);
	EXPECT_EQ(following_list[0], 4);
	following_list = grammar.Following(1);
	EXPECT_EQ(following_list.size(), 1);
	EXPECT_EQ(following_list[0], 4);


	// collections
	int collection_size = grammar.GenerateCollections();
	EXPECT_EQ(collection_size, 5);


	// closure
	std::vector<int> closure_size = {
		2, 0, 2, 0, 0
	};
	std::vector<ProductionItem<bool>*> core_items = {
		production_s_e->Item(0),
		production_s_e->Item(1),
		production_e_not_e->Item(1),
		production_e_not_e->Item(2),
		production_e_id->Item(1)
	};
	
	for (size_t i = 0; i < core_items.size(); ++i) {
		std::vector<ProductionItem<bool>*> closure;
		EXPECT_EQ(grammar.MakeClosure(core_items[i], closure), 0);
		EXPECT_EQ(closure.size(), closure_size[i])
			<< "closure " << i << " size error";

		if (closure.size() == 2) {
			EXPECT_EQ(closure[0], production_e_not_e->Item(0));
			EXPECT_EQ(closure[1], production_e_id->Item(0));
		}
	}


	std::vector<std::vector<int>> collection_goto = {
		{-1, 1, 2, 3, -1},		
		{-1, -1, -1, -1, -1},
		{-1, 4, 2, 3, -1},
		{-1, -1, -1, -1, -1},
		{-1, -1, -1, -1, -1}
	};
	for (size_t c = 0; c < collection_goto.size(); ++c) {
		for (size_t s = 0; s < symbol_list.size(); ++s) {
			EXPECT_EQ(grammar.CollectionGoto(c, s), collection_goto[c][s])
				<< "collection goto error, collection " << c
				<< ", symbol " << s << std::endl;
		}
	}



	delete identifier;
	delete op_not;
	delete production_set_s;
	delete production_set_e;
	delete production_s_e;
	delete production_e_not_e;
	delete production_e_id;
}



TEST(GrammarTest, FirstList) {
	// check add_multi_grammar
	// std::cout << "==check add multi grammar==" << std::endl;
	AddMultiGrammar add_multi_grammar;
	std::vector<Symbol*> symbol_list = add_multi_grammar.SymbolList();

	int set_size = add_multi_grammar.ProductionSetSize();
	ASSERT_EQ(set_size, 4) << "Set size error";

	for (int set = 0; set < set_size; set++) {
		std::vector<int> first_set = add_multi_grammar.First(set);
		EXPECT_EQ(
			first_set.size(),
			add_multi_grammar_first_list[set].size()
		);

		for (size_t i = 0; i < first_set.size(); ++i) {
			Symbol *symbol = symbol_list[first_set[i]];
			EXPECT_TRUE(SymbolInFirst(symbol, add_multi_grammar_first_list[i], add_multi_grammar_first_type[i]));
		}
	}


	// check arithmetic grammar
	// std::cout << "==check arithmetic grammar==" << std::endl;
	ArithmeticGrammar arithmetic_grammar;
	symbol_list = arithmetic_grammar.SymbolList();
	
	set_size = arithmetic_grammar.ProductionSetSize();
	ASSERT_EQ(set_size, 4) << "Set size error";

	for (int set = 0; set < set_size; set++) {
		std::vector<int> first_set = arithmetic_grammar.First(set);
		EXPECT_EQ(
			first_set.size(),
			arithmetic_grammar_first_list[set].size()
		);

		for (size_t i = 0; i < first_set.size(); ++i) {
			Symbol *symbol = symbol_list[first_set[i]];
			EXPECT_TRUE(SymbolInFirst(symbol, arithmetic_grammar_first_list[i], arithmetic_grammar_first_type[i]));
		}
	}


	// check logical grammar
	// std::cout << "==check logical grammar==" << std::endl;
	LogicalGrammar logical_grammar;
	symbol_list = logical_grammar.SymbolList();
	
	set_size = logical_grammar.ProductionSetSize();
	ASSERT_EQ(set_size, 3) << "Set size error";

	for (int set = 0; set < 1; set++) {
		std::vector<int> first_set = logical_grammar.First(set);
		EXPECT_EQ(
			first_set.size(),
			logical_grammar_first_list[set].size()
		);

		for (size_t i = 0; i < first_set.size(); ++i) {
			Symbol *symbol = symbol_list[first_set[i]];
			EXPECT_TRUE(
				SymbolInFirst(
					symbol, 
					logical_grammar_first_list[i], 
					logical_grammar_first_type[i]
				)
			);
		}
	}
}



TEST(GrammarTest, FollowList) {
	std::vector<Symbol*> symbol_list;
	int set_size;

	// std::cout << "==check add multi grammar==" << std::endl;
	// add multi grammar
	AddMultiGrammar add_multi_grammar;
	symbol_list = add_multi_grammar.SymbolList();
	set_size = add_multi_grammar.ProductionSetSize();

	for (int set = 0; set < set_size; set++) {
		std::vector<int> follow_list = add_multi_grammar.Following(set);
		
		EXPECT_EQ(
			follow_list.size(),
			add_multi_following_list[set].size()
		) << "folloing list size error: set " << set;

		for (size_t i = 0; i < follow_list.size(); ++i) {
			EXPECT_TRUE(
				SymbolInFollow(
					follow_list[i], 
					add_multi_following_list[set], 
					add_multi_following_type[set], 
					symbol_list
				)
			);
		}
	}


	// std::cout << "==check arithmetic grammar==" << std::endl;
	// airthmetic grammar
	ArithmeticGrammar arithmetic_grammar;
	symbol_list = arithmetic_grammar.SymbolList();
	set_size = arithmetic_grammar.ProductionSetSize();

	for (int set = 0; set < set_size; set++) {
		std::vector<int> follow_list = arithmetic_grammar.Following(set);
		
		EXPECT_EQ(
			follow_list.size(),
			arithmetic_following_list[set].size()
		) << "folloing list size error: set " << set;

		for (size_t i = 0; i < follow_list.size(); ++i) {
			EXPECT_TRUE(
				SymbolInFollow(
					follow_list[i], 
					arithmetic_following_list[set], 
					arithmetic_following_type[set], 
					symbol_list
				)
			);
		}
	}


	// std::cout << "==check logical grammar==" << std::endl;
	// logical grammar
	LogicalGrammar logical_grammar;
	symbol_list = logical_grammar.SymbolList();
	set_size = logical_grammar.ProductionSetSize();

	for (int set = 0; set < set_size; set++) {
		std::vector<int> follow_list = logical_grammar.Following(set);
		
		EXPECT_EQ(
			follow_list.size(),
			logical_following_list[set].size()
		) << "folloing list size error: set " << set;

		for (size_t i = 0; i < follow_list.size(); ++i) {
			EXPECT_TRUE(
				SymbolInFollow(
					follow_list[i], 
					logical_following_list[set], 
					logical_following_type[set], 
					symbol_list
				)
			);
		}
	}

}



TEST(TestGrammar, CollectionGoto) {
	int collection_size;
	int symbol_size;
	std::vector<Symbol*> symbol_list;

	// check add multi grammar
	std::cout << "==check add multi grammar==" << std::endl;
	AddMultiGrammar add_multi_grammar;
	collection_size = add_multi_grammar.GenerateCollections();
	symbol_list = add_multi_grammar.SymbolList();
	symbol_size = symbol_list.size();

	EXPECT_EQ(collection_size, add_multi_goto_table.size());
	EXPECT_EQ(symbol_size+1, add_multi_goto_table[0].size());
	
	for (int c = 0; c < collection_size; ++c) {
		for (int s = 0; s < symbol_size+1; ++s) {
			EXPECT_EQ(add_multi_grammar.CollectionGoto(c, s), add_multi_goto_table[c][s])
				<< " goto table error: collection " << c << ", symbol " << s;
		}
	}


	// // check arithmetic grammar
	// std::cout << "==check arithmetic grammar==" << std::endl;
	// ArithmeticGrammar arithmetic_grammar;
	// int collection_size = arithmetic_grammar.GenerateCollections();
	// std::vector<Symbol*> symbol_list = arithmetic_grammar.SymbolList();
	// int symbol_size = symbol_list.size();

	// EXPECT_EQ(collection_size, arithmetic_goto_table.size());
	// EXPECT_EQ(symbol_size+1, arithmetic_goto_table[0].size());
	
	// for (int c = 0; c < collection_size; ++c) {
	// 	for (int s = 0; s < symbol_size+1; ++s) {
	// 		EXPECT_EQ(arithmetic_grammar.CollectionGoto(c, s), arithmetic_goto_table[c][s])
	// 			<< " goto table error: collection " << c << ", symbol " << s;
	// 	}
	// }


	// check logical grammar
	std::cout << "==check logical grammar==" << std::endl;
	LogicalGrammar logical_grammar;
	collection_size = logical_grammar.GenerateCollections();
	symbol_list = logical_grammar.SymbolList();
	symbol_size = symbol_list.size();

	EXPECT_EQ(collection_size, logical_goto_table.size());
	EXPECT_EQ(symbol_size+1, logical_goto_table[0].size());
	
	for (int c = 0; c < collection_size; ++c) {
		for (int s = 0; s < symbol_size+1; ++s) {
			EXPECT_EQ(logical_grammar.CollectionGoto(c, s), logical_goto_table[c][s])
				<< " goto table error: collection " << c << ", symbol " << s;
		}
	}

}