#include "syntax/logical_grammar.h"

#include <vector>

#include "syntax/parser/production.h"
#include "syntax/parser/token.h"

namespace ecc {

LogicalGrammar::LogicalGrammar() noexcept
:Grammar<bool>() {

	// terminals
	Symbol *identifier = new Symbol(kSymbolType_Identifier);
	Operator *op_or = new Operator('|');
	Operator *op_and = new Operator('&');
	Operator *op_left_bracket = new Operator('(');
	Operator *op_right_bracket = new Operator(')');


	// non-terminals

	// production set
	ProductionFactorySet<bool> *production_set_s = new ProductionFactorySet<bool>;
	ProductionFactorySet<bool> *production_set_e = new ProductionFactorySet<bool>;
	ProductionFactorySet<bool> *production_set_t = new ProductionFactorySet<bool>;

	// single production
	ProductionFactory<bool> *production_s_e = new ProductionFactory<bool>(		// initial production
		production_set_s,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<bool>(symbols[0]);
		}
	);
	ProductionFactory<bool> *production_e_e_or_t = new ProductionFactory<bool>(
		production_set_e,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<bool>(symbols[0]) ||  Evaluate<bool>(symbols[2]);
		}
	);
	ProductionFactory<bool> *production_e_e_and_t = new ProductionFactory<bool>(
		production_set_e,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<bool>(symbols[0]) && Evaluate<bool>(symbols[2]);
		}
	);
	ProductionFactory<bool> *production_e_t = new ProductionFactory<bool>(
		production_set_e,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<bool>(symbols[0]);
		}
	);
	ProductionFactory<bool> *production_t_bracket_e = new ProductionFactory<bool>(
		production_set_t,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<bool>(symbols[1]);
		}
	);
	ProductionFactory<bool> *production_t_id = new ProductionFactory<bool>(
		production_set_t,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<bool>(symbols[0]);
		}
	);


	// add single production to production set
	// set s, start production set
	production_set_s->AddProductionFactory(production_s_e);
	// set e
	production_set_e->AddProductionFactory(production_e_e_or_t);
	production_set_e->AddProductionFactory(production_e_e_and_t);
	production_set_e->AddProductionFactory(production_e_t);
	// set t
	production_set_t->AddProductionFactory(production_t_bracket_e);
	production_set_t->AddProductionFactory(production_t_id);

	// add children to production to complete them
	production_s_e->SetChildren(production_set_e);
	production_e_e_or_t->SetChildren(production_set_e, op_or, production_set_t);
	production_e_e_and_t->SetChildren(production_set_e, op_and, production_set_t);
	production_e_t->SetChildren(production_set_t);
	production_t_bracket_e->SetChildren(op_left_bracket, production_set_e, op_right_bracket);
	production_t_id->SetChildren(identifier);


	// add sets to the grammar
	AddProductionSet(production_set_s, true);
	AddProductionSet(production_set_e);
	AddProductionSet(production_set_t);


	// store the pointers
	symbols_.push_back(identifier);
	symbols_.push_back(op_or);
	symbols_.push_back(op_and);
	symbols_.push_back(op_left_bracket);
	symbols_.push_back(op_right_bracket);
	symbols_.push_back(production_s_e);
	symbols_.push_back(production_e_e_or_t);
	symbols_.push_back(production_e_e_and_t);
	symbols_.push_back(production_e_t);
	symbols_.push_back(production_t_bracket_e);
	symbols_.push_back(production_t_id);
	symbols_.push_back(production_set_s);
	symbols_.push_back(production_set_e);
	symbols_.push_back(production_set_t);
}


LogicalGrammar::~LogicalGrammar() noexcept {
	for (auto s : symbols_) {
		delete s;
	}
}


}					// namespace ecc