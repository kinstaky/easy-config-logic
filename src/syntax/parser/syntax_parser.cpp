#include "syntax/parser/syntax_parser.h"

#include <iostream>
#include <stack>

#include "syntax/parser/grammar.h"
#include "syntax/parser/production.h"
#include "syntax/parser/token.h"


namespace ecl {

//-----------------------------------------------------------------------------
// 									ActionTable
//-----------------------------------------------------------------------------

ActionTable::ActionTable(int collection_size, int symbol_size) noexcept
: collection_size_(collection_size), symbol_size_(symbol_size) {
	size_ = collection_size * symbol_size;
	table_ = new struct Action[size_];
	for (int i = 0; i < size_; ++i) {
		table_[i].type = Action::kTypeError;
	}
}


ActionTable::~ActionTable() noexcept {
	delete[] table_;
}



int ActionTable::SetAction(int collection, int symbol, int type, int next) noexcept {
	int index = collection * symbol_size_ + symbol;
	if (index >= size_) return -1;
	table_[index].type = type;
	table_[index].collection = next;
	return 0; 
}


int ActionTable::SetAction(int collection, int symbol, int type, void *production) noexcept {
	int index = collection * symbol_size_ + symbol;
	if (index >= size_) return -1;
	table_[index].type = type;
	table_[index].production = production;
	return 0;
}


Action* ActionTable::GetAction(int collection, int symbol) noexcept {
	int index = collection * symbol_size_ + symbol;
	if (index >= size_) return nullptr;
	return table_ + index;
}




//-----------------------------------------------------------------------------
// 									SyntaxParser
//-----------------------------------------------------------------------------

template<typename VarType>
SyntaxParser<VarType>::SyntaxParser(Grammar<VarType> *grammar)
: grammar_(grammar) {
	if (!grammar->IsComplete()) {
		throw std::runtime_error("grammar not complete");
	}
	symbol_list_ = grammar->SymbolList();
}


template<typename VarType>
int SyntaxParser<VarType>::FindSymbol(TokenPtr token) noexcept {
	if (symbol_list_.empty()) {
		symbol_list_ = grammar_->SymbolList();
	}
	if (token->Type() < 0) return -1;
	for (size_t i = 0; i < symbol_list_.size(); ++i) {
		
		if (token->Type() != symbol_list_[i]->Type()) {
			continue;
		}
		
		if (
			token->Type() == kSymbolType_Variable
			|| token->Type() == kSymbolType_Literal
		) {
			return i;
		} else if (token->Type() == kSymbolType_Operator) {
			Operator *op = (Operator*)symbol_list_[i];
			if (token->Value() == op->Value()) {
				return i;
			}		
		} 
	}
	return -1;
}




template<typename VarType> 
int SyntaxParser<VarType>::AttachIdentifier(const std::string &name, void *var_ptr) noexcept {
	if (!var_ptr) return -1;
	for (Variable *var : variable_list_) {
		if (var->Value() == name) {
			// found the identifier match the name
			var->Attach(var_ptr);
			return 0;
		}
	}
	// not found return -1
	return -2;
}


template<typename VarType>
int SyntaxParser<VarType>::AttachIdentifier(size_t index, void *var_ptr) noexcept {
	if (!var_ptr) return -1;
	if (index >= variable_list_.size()) {
		return -2;
	}

	variable_list_[index]->Attach(var_ptr);
	return 0;
}



template<typename VarType>
int SyntaxParser<VarType>::PrintTree(Symbol *symbol, std::string prefix) const noexcept {
	// some unicode vertical and horizontal lines
	const std::string kBoxHorizontal = "\u2500";
	const std::string kBoxVertical = "\u2502";
	const std::string kBoxUpRight = "\u2514";
	const std::string kBoxVerticalRight = "\u251C";


	// print this token or production set first
	if (symbol->Type() > 0) {
		// print token
		std::cout << prefix << ((Token*)symbol)->Value() << std::endl;
	} else if (symbol->Type() == kSymbolType_Production) {
		// print production set 
		Production<VarType> *production = (Production<VarType>*)symbol;
		ProductionFactory<VarType> *factory = (ProductionFactory<VarType>*)production->Origin();
		ProductionFactorySet<VarType> *set = (ProductionFactorySet<VarType>*)factory->Parent();
		int set_index = grammar_->FindSet(set);
		std::cout << prefix << "S" << set_index << std::endl;

		// print its children
		if (production->size() > 0) {
			// genearte new prefix
			std::string children_prefix;
			for (size_t i = 0; i < prefix.length(); ++i) {
				if (prefix[i] ==  ' ') {
					// prefix is space if last line is space
					children_prefix +=  " ";
				} else if (prefix.substr(i, 3) == kBoxVertical || prefix.substr(i, 3) == kBoxVerticalRight) {
					// prefix is vertical line if last line contains vertical line
					children_prefix += kBoxVertical;
					i += 2;
				} else {
					// prefix is space if last line is up right or horizontal line
					children_prefix += " ";
					i += 2;
				}
			}
			// print children
			for (size_t i = 0; i < production->size()-1; ++i) {
				PrintTree(production->Child(i), children_prefix + kBoxVerticalRight + kBoxHorizontal);
			}
			PrintTree(production->Child(production->size()-1), children_prefix + kBoxUpRight + kBoxHorizontal);
		}

	} else {
		std::cerr << "Error: Invalid symbol type " << symbol->Type() << std::endl;
	}

	return 0;
}






//-----------------------------------------------------------------------------
// 								SLRSyntaxParser
//-----------------------------------------------------------------------------

template<typename VarType>
SLRSyntaxParser<VarType>::SLRSyntaxParser(Grammar<VarType> *grammar)
: SyntaxParser<VarType>(grammar) {

	// inititalize collection
	int collection_size = grammar->GenerateCollections(0);

	// initialize action table
	action_table_ = new ActionTable(collection_size, this->symbol_list_.size()+1);


// std::cout << "start add goto and shift action" << std::endl;
	// add goto and shift action
	for (int c = 0; c < collection_size; ++c) {
		for (size_t s = 0; s != this->symbol_list_.size(); ++s) {
			
			// get action value
			int collection = grammar->CollectionGoto(c, s);

// std::cout << "get action of goto collection " << collection << std::endl;
// std::cout << "  from collection " << c << " and meet symbol " << s << std::endl;

			if (collection < 0) {
				
				// invalid collection index, add error action
				action_table_->SetAction(c, s, Action::kTypeError, -1);

// std::cout << "set ERROR action from collection " << c << " meet symbol " << s << std::endl;

			} else {
				// check symbol type
				int stype = this->symbol_list_[s]->Type();
				if (
					stype == kSymbolType_Variable
					|| stype == kSymbolType_Operator
					|| stype == kSymbolType_Literal
				) {

					// shift action for identifier
					action_table_->SetAction(c, s, Action::kTypeShift, collection);

// std::cout << "set SHIFT " << collection << " action from collection " << c
// 	<< " meet symbol " << s << std::endl;

				} else if (stype == kSymbolType_ProductionFactorySet) {

					// goto action for production
					action_table_->SetAction(c, s, Action::kTypeGoto, collection);
				
// std::cout << "set GOTO " << collection << " action from collection " << c
// 	<< " meet symbol " << s << std::endl;

				} else {
					throw std::runtime_error("invalid symbol type in syntax list");
				}
			}
		}
	}



// std::cout << "start to add reduce and accept action" << std::endl;
	// add reduce and accept action
	for (int c = 0; c < collection_size; ++c) {
		auto collection = grammar->Collection(c);

		for (auto item : *collection) {

			int set = grammar->FindSet((ProductionFactorySet<VarType>*)item->Parent());

			if (item->IsLast()) {

				// accept action
				if (set == 0) {	// start production
					
					// supossed that the last symbol is terminating symbol '$'
					action_table_->SetAction(c, this->symbol_list_.size(), Action::kTypeAccept, nullptr);

				} else {

					// reduce action
					std::vector<int> follow_list = grammar->Following(set); 
					for (int symbol : follow_list) {
						action_table_->SetAction(c, symbol, Action::kTypeReduce, item->Origin());

// int reduce = 0;
// for (int i = 0; i < grammar->ProductionSetSize(); ++i) {
// 	bool found = false;
// 	ProductionFactorySet<VarType> *tmp_set = grammar->ProductionSet(i);
// 	for (int j = 0; j < tmp_set->size(); ++j) {
// 		if ((*tmp_set)[j] == item->Origin()) {
// 			reduce += j;
// 			found = true;
// 			break;
// 		}
// 	}
// 	if (found) break;
// 	reduce += tmp_set->size();
// }
// std::cout << "set REDUCE " << reduce << " action from collection " << c
// 	<< " meet symbol " << symbol << std::endl;

					}
				}
			}
		}
	}

// std::cout << "end of SLRSyntaxParser constructor" << std::endl;
}


template<typename VarType>
SLRSyntaxParser<VarType>::~SLRSyntaxParser() noexcept {
	for (auto &var : this->variable_list_) delete var;
	for (auto &literal : this->literal_list_) delete literal;
}


template<typename VarType>
int SLRSyntaxParser<VarType>::Parse(const std::vector<TokenPtr> &tokens) {

	// GenerateSyntaxTable(tokens);


	std::stack<int> collection_stack; 
	// inititalize
	collection_stack.push(0);


	// the looking symbol, maybe is in the token list or
	// is the result of reduce action
	int look_symbol = this->FindSymbol(tokens[0]);
	if (look_symbol < 0) {
		std::cerr << "Error: Invalid token " << tokens[0]->Value() << std::endl;
		return -1;			// invalid symbol
	}
	// the processing token index
	size_t itoken = 0;
	// processing symbols, includes the shift-in tokens and reduced productions
	std::stack<Symbol*> processing_symbols;

	// add identifier
	if (tokens[0]->Type() == kSymbolType_Variable) {
		this->variable_list_.push_back(new Variable(tokens[0]->Value()));
	}


	while (true) {

// std::cout << "== New loop, top of collection stack is " << collection_stack.top()
// 	<< ", looking symbol is " << look_symbol << std::endl;
		
		int top = collection_stack.top();
		Action *action = action_table_->GetAction(top, look_symbol);


		if (action->type == Action::kTypeShift || action->type == Action::kTypeGoto) {


// std::cout << "  Action " << (action->type == Action::kTypeShift ? "SHIFT " : "GOTO ")
// 	<< action->collection << " symbol " << look_symbol << std::endl;

			
			collection_stack.push(action->collection);
			

			// shift
			if (action->type == Action::kTypeShift) {
				// shift the looking symbol into the processing stack
				if (tokens[itoken]->Type() == kSymbolType_Variable) {
					// shift an identifier, find it in the identifier list
					Variable *id = nullptr;
					for (auto i : this->variable_list_) {
						if (i->Value() == tokens[itoken]->Value()) {
							id = i;
							break;
						}
					}
					if (!id) {
						std::cerr << "Error: The shifting identifier not found." << std::endl;
						return -1;
					}
					processing_symbols.push(id);
				} else if (tokens[itoken]->Type() == kSymbolType_Literal) {
					// shift a literal
					NumberLiteral *literal = new NumberLiteral(atoi(tokens[itoken]->Value().c_str()));
					this->literal_list_.push_back(literal);
					processing_symbols.push(literal);
				} else if (tokens[itoken]->Type() == kSymbolType_Operator) {
					// shift an operator, find it in the symbol list
					processing_symbols.push(this->symbol_list_[this->FindSymbol(tokens[itoken])]);
				}

				// also move to next token in the token list
				++itoken;
			}



			if (itoken == tokens.size()) {

// std::cout << "    Next symbol is FINISH symbol '$'" << std::endl;

				// the last token, get the terminating symbol
				look_symbol = this->symbol_list_.size();


			} else {

				// look for next token
				look_symbol = this->FindSymbol(tokens[itoken]);
				if (look_symbol < 0) {
					std::cerr << "Error: Invalid token " << tokens[itoken]->Value() << std::endl;
					return -1;			// invalid symbol
				}


				if (tokens[itoken]->Type() == kSymbolType_Variable) {
					
					Variable *variable = nullptr;
					// this token is identifier, check whether this has appeared before
					for (auto var : this->variable_list_) {
						if (var->Value() == tokens[itoken]->Value()) {

// std::cout << "    Next symbol is an EXISTING identifier." << std::endl;


							// found this identifier
							variable = var;
							break;
						}
					}

					if (!variable) {

// std::cout << "    Next symbol is a NEW identifier." << std::endl;

						// identifier not found, create a new one
						variable = new Variable(tokens[itoken]->Value());
						this->variable_list_.push_back(variable);
					}				

				} else if (tokens[itoken]->Type() == kSymbolType_Literal) {

				} else if (tokens[itoken]->Type() == kSymbolType_Operator) {

// std::cout << "    Next symbol is operator " << tokens[itoken]->Type() << std::endl;

				} else {
					std::cerr << "Error: Invalid token type " << tokens[itoken]->Type() << std::endl;
				}
			}
		

		} else if (action->type == Action::kTypeReduce) {

			ProductionFactory<VarType> *factory = (ProductionFactory<VarType>*)(action->production);

			// pop several collections
			for (size_t i = 0; i < factory->size(); ++i) {
				collection_stack.pop();
			}
			look_symbol = this->grammar_->FindSymbol(factory->Parent());



// int production_index = 0;
// for (int i = 0; i < this->grammar_->ProductionSetSize(); ++i) {
// 	bool found = false;
// 	ProductionFactorySet<VarType> *set = this->grammar_->ProductionSet(i);
// 	for (int j = 0; j < set->size(); ++j) {
// 		if ((*set)[j] == factory) {
// 			production_index += j;
// 			found = true;
// 		}
// 	}
// 	if (found) break;
// 	production_index += set->size();
// }
// std::cout << "  Action REDUCE, production index " << production_index
// 	<< ", production children size " << factory->size() 
// 	<< ", processing symbols size " << processing_symbols.size() << std::endl;


			// generate syntax tree
			Production<VarType> *production = factory->CreateProduction(processing_symbols);
			for (size_t i = 0; i < factory->size(); ++i) {
				processing_symbols.pop();
			}			
			processing_symbols.push(production);

		} else if (action->type == Action::kTypeAccept) {

// std::cout << "  Action ACCETP! Break the loop." << std::endl;

			this->syntax_tree_root_ = (Production<VarType>*)(processing_symbols.top());
			break;


		} else {
			
			std::cerr << "Error: Invalid action type: " << action->type
				<< ", stack top symbol is " << top << ", next symbol is "
				<< tokens[itoken]->Value() << "\n";
			return -2;
		}

	}


// this->PrintTree(this->syntax_tree_root_);

	return 0;
}


//-----------------------------------------------------------------------------
//					explicit instantiations of template classes
//-----------------------------------------------------------------------------

template class SyntaxParser<bool>;
template class SyntaxParser<int>;
template class SyntaxParser<double>;

template class SLRSyntaxParser<bool>;
template class SLRSyntaxParser<int>;
template class SLRSyntaxParser<double>;

} 				// namespace ecl