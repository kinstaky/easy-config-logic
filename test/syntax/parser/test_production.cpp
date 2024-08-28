#include "syntax/parser/production.h"

#include <gtest/gtest.h>

#include "syntax/parser/token.h"

using namespace ecl;


TEST(ProductionTest, Children) {
	Production<bool> *root = new Production<bool>(nullptr, 1, nullptr);
	Production<bool> *leaf = new Production<bool>(root, 2, nullptr);
	Operator *op_start = new Operator("^");
	Operator *op_end = new Operator("&");

	// set children
	root->SetChild(0, leaf);
	leaf->SetChildren(op_start, op_end);

	// check children size
	EXPECT_EQ(root->size(), 1) << "root size error";
	EXPECT_EQ(leaf->size(), 2) << "leaf size error";
	
	// check children pointer
	Symbol* child = root->Child(0);
	EXPECT_EQ(child, leaf) << "root child 0 error";
	child = leaf->Child(0);
	EXPECT_EQ(child, op_start) << "leaf child 0 error";
	child = leaf->Child(1);
	EXPECT_EQ(child, op_end) << "leaf child 1 error";

	// check parent
	Symbol *parent = root->Parent();
	EXPECT_EQ(parent, nullptr) << "root parent error";
	parent = leaf->Parent();
	EXPECT_EQ(parent, root) << "leaf parent error";
}



TEST(ProductionTest, ProductionFactorySet) {
	Operator* op_add = new Operator("+");

	ProductionFactorySet<double> *production_set_e = new ProductionFactorySet<double>;

	ProductionFactory<double> *production_e_e_add = new ProductionFactory<double>(
		(ProductionBase*)production_set_e,
		2,
		nullptr
	);

	production_set_e->AddProductionFactory(production_e_e_add);

	production_e_e_add->SetChildren(production_set_e, op_add);


	EXPECT_EQ(production_set_e->size(), 1);
	EXPECT_TRUE(production_set_e->IsComplete());
	EXPECT_EQ((*production_set_e)[0], production_e_e_add);


	delete op_add;
	delete production_e_e_add;
	delete production_set_e;
}




TEST(ProductionTest, ProductionFactory) {
	Symbol *identifier = new Symbol(kSymbolType_Variable);
	Operator *op_add = new Operator("+");

	ProductionFactorySet<double> *production_set_e = new ProductionFactorySet<double>;

	ProductionFactory<double> *production_e_e_add_id = new ProductionFactory<double>(
		production_set_e,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]) + Evaluate<double>(symbols[2]);
		}		
	);
	ProductionFactory<double> *production_e_id = new ProductionFactory<double>(
		production_set_e,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]);
		}
	);

	production_e_e_add_id->SetChildren(production_set_e, op_add, identifier);
	production_e_id->SetChildren(identifier);


	production_set_e->AddProductionFactory(production_e_e_add_id);
	production_set_e->AddProductionFactory(production_e_id);

	// create production
	std::stack<Symbol*> symbols;
	
	Variable *a = new Variable("A");
	Variable *b = new Variable("B");


	symbols.push(a);
	Production<double>* production1 = production_e_id->CreateProduction(symbols);

	symbols.pop();
	symbols.push(production1);
	symbols.push(op_add);
	symbols.push(b);

	Production<double>* production2 = production_e_e_add_id->CreateProduction(symbols);


	// check 
	EXPECT_EQ(production1->size(), 1);
	EXPECT_EQ(production1->Child(0), a);

	EXPECT_EQ(production2->size(), 3);
	EXPECT_EQ(production2->Child(0), production1);
	EXPECT_EQ(production2->Child(1), op_add);
	EXPECT_EQ(production2->Child(2), b);


	double var_a = 20.0;
	double var_b = 0.5;
	a->Attach(&var_a);
	b->Attach(&var_b);

	EXPECT_EQ(production2->Eval(), 20.5);



	delete identifier;
	delete op_add;
	delete production_set_e;
	delete production_e_e_add_id;
	delete a;
	delete b;
}


TEST(ProductionTest, GenearteItems) {
	Symbol *identifier = new Symbol(kSymbolType_Variable);
	Operator *op_add = new Operator("+");

	ProductionFactorySet<double> *production_set_e = new ProductionFactorySet<double>;

	ProductionFactory<double> *production_e_e_add_id = new ProductionFactory<double>(
		production_set_e,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]) + Evaluate<double>(symbols[2]);
		}		
	);
	ProductionFactory<double> *production_e_id = new ProductionFactory<double>(
		production_set_e,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]);
		}
	);

	production_e_e_add_id->SetChildren(production_set_e, op_add, identifier);
	production_e_id->SetChildren(identifier);

	production_set_e->AddProductionFactory(production_e_e_add_id);
	production_set_e->AddProductionFactory(production_e_id);


	// create items
	ProductionFactory<double> *productions[2] = {
		production_e_e_add_id,
		production_e_id
	};
	for (auto production : productions) {
		size_t index = 0;
		while  (auto item = production->GenerateItems()) {
			EXPECT_EQ(item->size(), production->size());
			EXPECT_EQ(item->Parent(), production->Parent());
			for (size_t c = 0; c < item->size(); ++c) {
				EXPECT_EQ(item->Child(c), production->Child(c));
			}
			EXPECT_EQ(item->Origin(), production);
			EXPECT_EQ(item->Index(), index);
			EXPECT_EQ(item->IsLast(), item->size() == index);
			if (!item->IsLast()) {
				EXPECT_EQ(item->ExpectedSymbol(), item->Child(index));
			}
			++index;
		}

	}
}




TEST(ProductionTest, ProductionCollection) {
	Symbol *identifier = new Symbol(kSymbolType_Variable);
	Operator *op_add = new Operator("+");	

	ProductionFactorySet<double> *production_set_e = new ProductionFactorySet<double>();

	ProductionFactory<double> *production_e_e_add_id = new ProductionFactory<double>(
		production_set_e,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]) + Evaluate<double>(symbols[2]);
		}
	);
	production_e_e_add_id->SetChildren(production_set_e, op_add, identifier);

	ProductionItemCollection<double> *collection = new ProductionItemCollection<double>;
	std::vector<ProductionItem<double>*> items;


	while (auto item = production_e_e_add_id->GenerateItems()) {
		collection->AddItem(item, true);
		items.push_back(item);
	}


	EXPECT_EQ(collection->size(), 4);

	int index = 0;
	for (auto item : *collection) {
		EXPECT_EQ(item, items[index]);
		++index;
	}

	
	collection->SetGoto(op_add, collection);
	EXPECT_EQ(collection->Goto(op_add), collection);
}