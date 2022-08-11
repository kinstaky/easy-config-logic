#include "syntax/arithmetic_grammar.h"

#include <vector>

#include "syntax/parser/grammar.h"
#include "syntax/parser/production.h"
#include "syntax/parser/token.h"

namespace ecl {

//-----------------------------------------------------------------------------
//	 				Add_Multi_Grammar
//-----------------------------------------------------------------------------


AddMultiGrammar::AddMultiGrammar() noexcept
: Grammar<double>() {

	// terminal symbols
	Symbol *identifier = new Symbol(kSymbolType_Identifier);
	Operator *op_add = new Operator("+");
	Operator *op_multi = new Operator("*");
	Operator *op_left_bracket = new Operator("(");
	Operator *op_right_bracket = new Operator(")");

	// non-terminal symbols

	// production set
	ProductionFactorySet<double> *production_set_s = new ProductionFactorySet<double>;
	ProductionFactorySet<double> *production_set_e = new ProductionFactorySet<double>;
	ProductionFactorySet<double> *production_set_t = new ProductionFactorySet<double>;
	ProductionFactorySet<double> *production_set_f = new ProductionFactorySet<double>;

	// single production
	ProductionFactory<double> *production_s_e = new ProductionFactory<double>(
		production_set_s,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]);
		}
	);
	ProductionFactory<double> *production_e_e_add_t = new ProductionFactory<double>(
		production_set_e,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]) + Evaluate<double>(symbols[2]);
		}
	);
	ProductionFactory<double> *production_e_t = new ProductionFactory<double>(
		production_set_e,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]);
		}
	);
	ProductionFactory<double> *production_t_t_multi_f = new ProductionFactory<double>(
		production_set_t,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]) * Evaluate<double>(symbols[2]);
		}
	);
	ProductionFactory<double> *production_t_f = new ProductionFactory<double>(
		production_set_t,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]);
		}
	);
	ProductionFactory<double> *production_f_bracket_e = new ProductionFactory<double>(
		production_set_f,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[1]);
		}
	);
	ProductionFactory<double> *production_f_id = new ProductionFactory<double>(
		production_set_f,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]);
		}
	);

	// add single production to production set
	// set s, start production set
	production_set_s->AddProductionFactory(production_s_e);
	// set e
	production_set_e->AddProductionFactory(production_e_e_add_t);
	production_set_e->AddProductionFactory(production_e_t);
	// set t
	production_set_t->AddProductionFactory(production_t_t_multi_f);
	production_set_t->AddProductionFactory(production_t_f);
	// set f
	production_set_f->AddProductionFactory(production_f_bracket_e);
	production_set_f->AddProductionFactory(production_f_id);


	// add children to production factories
	production_s_e->SetChildren(production_set_e);
	production_e_e_add_t->SetChildren(production_set_e, op_add, production_set_t);
	production_e_t->SetChildren(production_set_t);
	production_t_t_multi_f->SetChildren(production_set_t, op_multi, production_set_f);
	production_t_f->SetChildren(production_set_f);
	production_f_bracket_e->SetChildren(op_left_bracket, production_set_e, op_right_bracket);
	production_f_id->SetChildren(identifier);

	// add sets to the grammar
	AddProductionSet(production_set_s, true);
	AddProductionSet(production_set_e);
	AddProductionSet(production_set_t);
	AddProductionSet(production_set_f);

	// store the pointers
	symbols_.push_back(identifier);
	symbols_.push_back(op_add);
	symbols_.push_back(op_multi);
	symbols_.push_back(op_left_bracket);
	symbols_.push_back(op_right_bracket);
	symbols_.push_back(production_s_e);
	symbols_.push_back(production_e_e_add_t);
	symbols_.push_back(production_e_t);
	symbols_.push_back(production_t_t_multi_f);
	symbols_.push_back(production_t_f);
	symbols_.push_back(production_f_bracket_e);
	symbols_.push_back(production_f_id);
	symbols_.push_back(production_set_s);
	symbols_.push_back(production_set_e);
	symbols_.push_back(production_set_t);
	symbols_.push_back(production_set_f);
}



AddMultiGrammar::~AddMultiGrammar() {
	for (auto symbol : symbols_) {
		delete symbol;
	}
}



//-----------------------------------------------------------------------------
//	 			Arithmetic_Grammar
//-----------------------------------------------------------------------------

ArithmeticGrammar::ArithmeticGrammar() noexcept
:Grammar<double>() {

	// terminal symbols
	Symbol *identifier = new Symbol(kSymbolType_Identifier);
	Operator *op_add = new Operator("+");
	Operator *op_sub = new Operator("-");
	Operator *op_mul = new Operator("*");
	Operator *op_div = new Operator("/");
	Operator *op_left_bracket = new Operator("(");
	Operator *op_right_bracket = new Operator(")");

	// non-terminal symbols

	// production set
	ProductionFactorySet<double> *production_set_s = new ProductionFactorySet<double>;
	ProductionFactorySet<double> *production_set_e = new ProductionFactorySet<double>;
	ProductionFactorySet<double> *production_set_t = new ProductionFactorySet<double>;
	ProductionFactorySet<double> *production_set_f = new ProductionFactorySet<double>;

	// single production
	ProductionFactory<double> *production_s_e = new ProductionFactory<double>(
		production_set_s,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]);
		}
	);
	ProductionFactory<double> *production_e_e_add_t = new ProductionFactory<double>(
		production_set_e,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]) + Evaluate<double>(symbols[2]);
		}
	);
	ProductionFactory<double> *production_e_e_sub_t = new ProductionFactory<double>(
		production_set_e,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]) - Evaluate<double>(symbols[2]);
		}
	);
	ProductionFactory<double> *production_e_t = new ProductionFactory<double>(
		production_set_e,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]);
		}
	);
	ProductionFactory<double> *production_t_t_mul_f = new ProductionFactory<double>(
		production_set_t,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]) * Evaluate<double>(symbols[2]);
		}
	);
	ProductionFactory<double> *production_t_t_div_f = new ProductionFactory<double>(
		production_set_t,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]) / Evaluate<double>(symbols[2]);
		}
	);
	ProductionFactory<double> *production_t_f = new ProductionFactory<double>(
		production_set_t,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]);
		}
	);
	ProductionFactory<double> *production_f_bracket_e = new ProductionFactory<double>(
		production_set_f,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[1]);
		}
	);
	ProductionFactory<double> *production_f_id = new ProductionFactory<double>(
		production_set_f,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<double>(symbols[0]);
		}
	);

	// add single production to production set
	// set s, start production set
	production_set_s->AddProductionFactory(production_s_e);
	// set e
	production_set_e->AddProductionFactory(production_e_e_add_t);
	production_set_e->AddProductionFactory(production_e_e_sub_t);
	production_set_e->AddProductionFactory(production_e_t);
	// set t
	production_set_t->AddProductionFactory(production_t_t_mul_f);
	production_set_t->AddProductionFactory(production_t_t_div_f);
	production_set_t->AddProductionFactory(production_t_f);
	// set f
	production_set_f->AddProductionFactory(production_f_bracket_e);
	production_set_f->AddProductionFactory(production_f_id);

	// add children to production factories
	production_s_e->SetChildren(production_set_e);
	production_e_e_add_t->SetChildren(production_set_e, op_add, production_set_t);
	production_e_e_sub_t->SetChildren(production_set_e, op_sub, production_set_t);
	production_e_t->SetChildren(production_set_t);
	production_t_t_mul_f->SetChildren(production_set_t, op_mul, production_set_f);
	production_t_t_div_f->SetChildren(production_set_t, op_div, production_set_f);
	production_t_f->SetChildren(production_set_f);
	production_f_bracket_e->SetChildren(op_left_bracket, production_set_e, op_right_bracket);
	production_f_id->SetChildren(identifier);

	// add sets to the grammar
	AddProductionSet(production_set_s);
	AddProductionSet(production_set_e);
	AddProductionSet(production_set_t);
	AddProductionSet(production_set_f);

	// store the pointers
	symbols_.push_back(identifier);
	symbols_.push_back(op_add);
	symbols_.push_back(op_mul);
	symbols_.push_back(op_left_bracket);
	symbols_.push_back(op_right_bracket);
	symbols_.push_back(production_s_e);
	symbols_.push_back(production_e_e_add_t);
	symbols_.push_back(production_e_e_sub_t);
	symbols_.push_back(production_e_t);
	symbols_.push_back(production_t_t_mul_f);
	symbols_.push_back(production_t_t_div_f);
	symbols_.push_back(production_t_f);
	symbols_.push_back(production_f_bracket_e);
	symbols_.push_back(production_f_id);
	symbols_.push_back(production_set_s);
	symbols_.push_back(production_set_e);
	symbols_.push_back(production_set_t);
	symbols_.push_back(production_set_f);
}


ArithmeticGrammar::~ArithmeticGrammar() noexcept {
	for (auto symbol : symbols_) {
		delete symbol;
	}
}


}					// namespace ecl