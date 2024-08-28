#include "syntax/logic_downscale_grammar.h"

namespace ecl {

inline int Max(int a, int b) {
	return a > b ? a : b;
}

LogicDownscaleGrammar::LogicDownscaleGrammar() noexcept
: Grammar<int>() {

	// terminals
	Symbol *variable = new Symbol(kSymbolType_Variable);
	symbols_.push_back(variable);
	Symbol *op_equal = new Operator('=');
	symbols_.push_back(op_equal);
	Symbol *op_or = new Operator('|');
	symbols_.push_back(op_or);
	Symbol *op_and = new Operator('&');
	symbols_.push_back(op_and);
	Symbol *op_left_bracket = new Operator('(');
	symbols_.push_back(op_left_bracket);
	Symbol *op_right_bracket = new Operator(')');
	symbols_.push_back(op_right_bracket);
	Symbol *op_div = new Operator('/');
	symbols_.push_back(op_div);
	Symbol *digits = new Symbol(kSymbolType_Literal);
	symbols_.push_back(digits);

	// non-terminals
	// production sets
	ProductionFactorySet<int> *production_set_s = new ProductionFactorySet<int>;
	symbols_.push_back(production_set_s);
	ProductionFactorySet<int> *production_set_l = new ProductionFactorySet<int>;
	symbols_.push_back(production_set_l);
	ProductionFactorySet<int> *production_set_e = new ProductionFactorySet<int>;
	symbols_.push_back(production_set_e);
	ProductionFactorySet<int> *production_set_t = new ProductionFactorySet<int>;
	symbols_.push_back(production_set_t);
	ProductionFactorySet<int> *production_set_f = new ProductionFactorySet<int>;
	symbols_.push_back(production_set_f);

	// 0. S -> L
	ProductionFactory<int> *production_s_l = new ProductionFactory<int>(
		production_set_s,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<int>(symbols[0]);
		}
	);
	production_set_s->AddProductionFactory(production_s_l);
	production_s_l->SetChildren(production_set_l);
	symbols_.push_back(production_s_l);

	// 1. L -> id = E
	ProductionFactory<int> *production_l_id_equal_e = new ProductionFactory<int>(
		production_set_l,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<int>(symbols[2]);
		}
	);
	production_set_l->AddProductionFactory(production_l_id_equal_e);
	production_l_id_equal_e->SetChildren(variable, op_equal, production_set_e);
	symbols_.push_back(production_l_id_equal_e);

	// 2. E -> E | T
	ProductionFactory<int> *production_e_e_or_t = new ProductionFactory<int>(
		production_set_e,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Max(Evaluate<int>(symbols[0]), Evaluate<int>(symbols[2]));
		}
	);
	production_set_e->AddProductionFactory(production_e_e_or_t);
	production_e_e_or_t->SetChildren(production_set_e, op_or, production_set_t);
	symbols_.push_back(production_e_e_or_t);


	// 3. E -> E & T
	ProductionFactory<int> *production_e_e_and_t = new ProductionFactory<int>(
		production_set_e,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Max(Evaluate<int>(symbols[0]), Evaluate<int>(symbols[2]));
		}
	);
	production_set_e->AddProductionFactory(production_e_e_and_t);
	production_e_e_and_t->SetChildren(production_set_e, op_and, production_set_t);
	symbols_.push_back(production_e_e_and_t);

	// 4. E -> T
	ProductionFactory<int> *production_e_t = new ProductionFactory<int>(
		production_set_e,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<int>(symbols[0]);
		}
	);
	production_set_e->AddProductionFactory(production_e_t);
	production_e_t->SetChildren(production_set_t);
	symbols_.push_back(production_e_t);

	// 5. T -> F / digits
	ProductionFactory<int> *production_t_f_div_digits = new ProductionFactory<int>(
		production_set_t,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<int>(symbols[0]) + 1;
		}
	);
	production_set_t->AddProductionFactory(production_t_f_div_digits);
	production_t_f_div_digits->SetChildren(production_set_f, op_div, digits);
	symbols_.push_back(production_t_f_div_digits);

	// 6. T -> F
	ProductionFactory<int> *production_t_f = new ProductionFactory<int>(
		production_set_t,
		1,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<int>(symbols[0]);
		}
	);
	production_set_t->AddProductionFactory(production_t_f);
	production_t_f->SetChildren(production_set_f);
	symbols_.push_back(production_t_f);

	// 7. F -> id
	ProductionFactory<int> *production_f_id = new ProductionFactory<int>(
		production_set_f,
		1,
		[](const std::vector<Symbol*> &) {
			return 0;
		}
	);
	production_set_f->AddProductionFactory(production_f_id);
	production_f_id->SetChildren(variable);
	symbols_.push_back(production_f_id);

	// 8. F -> literal
	ProductionFactory<int> *production_f_literal = new ProductionFactory<int>(
		production_set_f,
		1,
		[](const std::vector<Symbol*> &) {
			return 0;
		}
	);
	production_set_f->AddProductionFactory(production_f_literal);
	production_f_literal->SetChildren(digits);
	symbols_.push_back(production_f_literal);

	// 9. F -> (E)
	ProductionFactory<int> *production_f_bracket_e = new ProductionFactory<int>(
		production_set_f,
		3,
		[](const std::vector<Symbol*> &symbols) {
			return Evaluate<int>(symbols[1]);
		}
	);
	production_set_f->AddProductionFactory(production_f_bracket_e);
	production_f_bracket_e->SetChildren(op_left_bracket, production_set_e, op_right_bracket);
	symbols_.push_back(production_f_bracket_e);


	AddProductionSet(production_set_s, true);
	AddProductionSet(production_set_l);
	AddProductionSet(production_set_e);
	AddProductionSet(production_set_t);
	AddProductionSet(production_set_f);
}

};