/*
 * This file is part of the context-free grammar library.
 */ 


#include "syntax/parser/grammar.h"

#include <iostream>

#include "syntax/parser/production.h" 
#include "syntax/parser/token.h"

namespace ecl {

//-----------------------------------------------------------------------------
// 						Grammar
//-----------------------------------------------------------------------------

template<typename VarType>
Grammar<VarType>::Grammar() noexcept {
	generate_following_ = false;
	generate_first_ = false;
}


template<typename VarType>
Grammar<VarType>::~Grammar() noexcept {
	for (auto c : collections_) delete c;
	for (auto s : symbols_) delete s;
}


template<typename VarType>
int Grammar<VarType>::AddProductionSet(ProductionFactorySet<VarType> *set, bool start) noexcept {
	if (start) {
		if (set->size() != 1) return -1;
		production_sets_.insert(production_sets_.begin(), set);		
	} else {
		production_sets_.push_back(set);
	}

	// check if exists in the symbol list
	// and add them to the symbol list
	if (FindSymbol(set) < 0) {
		symbol_list_.push_back(set);
		generate_following_ = false;
		generate_first_ = false;
	}

	for (auto factory : *set) {
		if (!factory) continue;
		for (size_t i = 0; i != factory->size(); ++i) {
			Symbol *symbol = factory->Child(i);
			if (symbol && FindSymbol(symbol) < 0) {
				symbol_list_.push_back(symbol);
				generate_following_ = false;
				generate_first_ = false;
			}
		}
	}

	return 0;
}



template<typename VarType>
bool Grammar<VarType>::IsComplete() noexcept {
	for (auto set : production_sets_) {
		bool result = set->IsComplete();		
		if (!result) return false;
	}
	return true;
}


template<typename VarType>
int Grammar<VarType>::GenerateCollections(int look_ahead) noexcept {
	if (look_ahead == 1) {
		std::cerr << "look ahead 1 todo." << std::endl;
		return -1;
	}
	// generate items
	for (ProductionFactorySet<VarType>* set : production_sets_) {
		for (ProductionFactory<VarType>* production : *set) {
			ProductionItem<VarType> *item = production->GenerateItems();

			if (core_items_.empty()) {
				// the first item from the first production factory set is 
				// the first item of the start production, it must be the 
				// core item

				core_items_.push_back(item);

			} else {
				// otherwise, all the first item of the other production
				// is non-core items



				non_core_items_.push_back(item);
			}


			// and all the other items (except for the first itme) from
			// the production are core items
			while ((item = production->GenerateItems())) {
				core_items_.push_back(item);
			}
		}
	
	}

// std::cout << "core items size " << core_items_.size() << std::endl;
// std::cout << "non core items size " << non_core_items_.size() << std::endl;

	// first collection
	collections_.push_back(new ProductionItemCollection<VarType>);
	collections_[0]->AddItem(core_items_[0], true);

	// non core items
	std::vector<ProductionItem<VarType>*> closure;
	MakeClosure(core_items_[0], closure);
	for (auto closure_item : closure) {
		collections_[0]->AddItem(closure_item, false);
	}

// std::cout << "collection 0 closure size " << closure.size() << std::endl;


	bool add_item = true;
	while (add_item) {
		add_item = false;
		for (size_t ic = 0; ic < collections_.size(); ++ic) {



// std::cout << "collections size " << collections_.size()
// 	<< ", searching collection index " << ic << std::endl;

			for (auto item : *(collections_[ic])) {

				Symbol *next_symbol;
				ProductionItem<VarType> *next_item = NextItem(item, &next_symbol);

// std::cout << "  item XX" << std::endl;				

				
				if (next_item) {
					// the next item is valid, the next step is to search for
					// the collection that includes this item.

// std::cout << "    next item valid, next symbol type " << next_symbol->Type();
// if (next_symbol->Type() == -3) {
// 	std::cout << ", set index " << FindSet((ProductionFactorySet<VarType>*)next_symbol);
// } else if (next_symbol->Type() > 0) {
// 	std::cout << ", token value "
// 		<< (next_symbol->Type() == 1 ? "id" : ((Token*)next_symbol)->Value());
// }
// std::cout << ", next item item_index " << next_item->Index() << std::endl;



					ProductionItemCollection<VarType> *collection = collections_[ic]->Goto(next_symbol);
					if (collection) {


// std::cout << "      collection found" << std::endl;

						// found the collection, check whether this item is in
						// this collection
						bool found = false;
						for (auto i : *collection) {
							if (next_item == i) {
								found = true;
								break;
							}
						}


// std::cout << "      next_item " << (found ? "" : "not ") << "found in collection. ";
// if (found) {
// 	int add_collection = -1;
// 	for (int i = 0; i < collections_.size(); ++i) {
// 		if (collection == collections_[i]) {
// 			add_collection = i;
// 			break;
// 		}
// 	}
// 	std::cout <<" Add item to collection " << add_collection << std::endl;
// }			


						if (!found) {
							collection->AddItem(next_item, true);
							add_item = true;
						}

					} else {
						// Cannot find the collection. There are two cases,
						// first is the collection has not been created,
						// second is the goto table has not been set
						// The next step is to search for probably exist
						// collection.



						for (auto c : collections_) {
							for (auto i : *c) {
								if (i == next_item) {
									// found the collection
									collection = c;
									break;
								}
							}
						}

						if (collection) {

							// collection has been created, add it to the goto
							// table
							collections_[ic]->SetGoto(next_symbol, collection);
							add_item = true;


// int add_collection = -1;
// for (int i = 0; i < collections_.size(); ++i) {
// 	if (collection == collections_[i]) {
// 		add_collection = i;
// 		break;
// 	}
// }
// std::cout << "        collection " << add_collection
// 	 << " has been created and set the goto table." << std::endl;



						} else {
							// collection has not been created, create it first
							collection = new ProductionItemCollection<VarType>;
							collections_.push_back(collection);
							collections_[ic]->SetGoto(next_symbol, collection);

							// add core item to collection
							collection->AddItem(next_item, true);


// int add_collection = -1;
// for (int i = 0; i < collections_.size(); ++i) {
// 	if (collection == collections_[i]) {
// 		add_collection = i;
// 		break;
// 	}
// }
// std::cout << "        create collection " << collections_.size() << std::endl;
// std::cout << "        Add item, add to collection " << add_collection << std::endl;


							// add non-core item to the collection
							std::vector<ProductionItem<VarType>*> closure;
							MakeClosure(next_item, closure);
							for (auto closure_item : closure) {
								collection->AddItem(closure_item, false);
							}

// std::cout << "    collection " << add_collection << " closure size " << closure.size() << std::endl;

							add_item = true;
						}
			

					}
				}
			}
		}
	}


	return collections_.size();
}




template<typename VarType>
int Grammar<VarType>::MakeClosure(ProductionItem<VarType> *item, std::vector<ProductionItem<VarType>*> &closure) noexcept {
	closure.clear();
	Symbol *symbol = item->ExpectedSymbol();
	for (auto it : non_core_items_) {
		if (it->Parent() == symbol) {
			closure.push_back(it);
		} 		
	}

	for (size_t closure_index = 0; closure_index < closure.size(); ++closure_index) {
		
		// loop the closure and finding items of the closure items 
		Symbol *symbol = closure[closure_index]->ExpectedSymbol();
		if (!symbol) {
			continue;
		}
		
		// search from the non-core items of the closure item
		for (auto it : non_core_items_) {
			if (it->Parent() == symbol) {
				// found appopriate item

				bool found = false;
				for (auto i : closure) {
					// check whether this item is already in the closure
					if (it == i) {
						found = true;
						break;
					}
				}
				if (!found) {
					// not found, it's ok to to push it into the closure
					closure.push_back(it);
				}
			}
		}
	}

	return 0;
}




template<typename VarType>
ProductionItem<VarType>* Grammar<VarType>::NextItem(ProductionItem<VarType> *item, Symbol **symbol) noexcept {
	if (item->IsLast()) return nullptr;
	*symbol = item->ExpectedSymbol();
	return item->Origin()->Item(item->Index()+1);
}



template<typename VarType>
int Grammar<VarType>::CollectionGoto(size_t collection, size_t symbol) noexcept {
	if (collection >= collections_.size()) return -1;
	if (symbol >= symbol_list_.size()) return -1;

	auto c = collections_[collection]->Goto(symbol_list_[symbol]);
	for (size_t ic = 0; ic < collections_.size(); ++ic) {
		if (collections_[ic] == c) {
			return ic;
		}
	}
	return -1;
}


template<typename VarType>
int Grammar<VarType>::FindSet(ProductionFactorySet<VarType> *set) noexcept {
	for (size_t s = 0; s < production_sets_.size(); ++s) {
		if (production_sets_[s] == set) {
			return s;
		}
	}
	return -1;
}


template<typename VarType>
int Grammar<VarType>::FindSymbol(Symbol *symbol) noexcept {
	for (size_t s = 0; s < symbol_list_.size(); ++s) {
		if (symbol->Type() == kSymbolType_Variable) {
			if (symbol_list_[s]->Type() == kSymbolType_Variable) {
				return s;
			}
		} else if (symbol_list_[s] == symbol) {
			return s;
		}
	}
	return -1;
}


template<typename VarType>
std::vector<Symbol*> Grammar<VarType>::SymbolList() const noexcept {
	return symbol_list_;
}



template<typename VarType>
std::vector<int> Grammar<VarType>::First(size_t set) noexcept {
	if (!generate_first_) {

		if (GenerateFirst() == -1) {

			return std::vector<int>(1, -1);
		}

	}
	if (set >= production_sets_.size()) {

		return std::vector<int>(1, -1);
	}

	std::vector<int> result;
	for (int i : first_list_[set]) {

		result.push_back(i);
	}

	return result;
}


template<typename VarType>
std::vector<int> Grammar<VarType>::First(ProductionFactory<VarType> *production, size_t child) noexcept {
	if (!generate_first_) {
		if (GenerateFirst() == -1) {
			return std::vector<int>(1, -1);
		}
	}
	if (child >= production->size()) {
		return std::vector<int>(1, -1);
	}

	// result
	std::set<int> result_set;
	for (size_t c = 0; c < production->size(); ++c) {
		Symbol *symbol = production->Child(c);

		if (symbol->Type() > 0) {

			// terminal symbol, first cannot be empty
			// insert if not exists in result
			auto search = result_set.find(FindSymbol(symbol));
			if (search == result_set.end()) {
				result_set.insert(FindSymbol(symbol));
			}
			break;			// no need to check next child

		} else if (symbol->Type() == kSymbolType_ProductionFactorySet) {

			// non-terminal symbol
			for (auto first : first_list_[FindSet((ProductionFactorySet<VarType>*)symbol)]) {
				// insert if not exists in result
				auto search = result_set.find(first);
				if (search == result_set.end()) {
					result_set.insert(first);
				}
			}

			// check empty symbol
			int check = FirstIncludeEmpty(FindSet((ProductionFactorySet<VarType>*)symbol));
			if (check < 0) {
				return std::vector<int>(1, -1);
			}
			// this child's first cannot be empty, no need to check next child
			if (!check) break;		
		
		} else {

			return std::vector<int>(1, -1);
		}
	}

	std::vector<int> result;
	for (int i : result_set) {
		result.push_back(i);
	}
	return result;
}


template<typename VarType>
int Grammar<VarType>::FirstIncludeEmpty(size_t set) noexcept {
	if (!generate_first_) {
		if (GenerateFirst() == -1) {
			return -1;
		}
	}
	if (set >= production_sets_.size()) {
		return -1;
	}
	return first_include_empty_[set] ? 1 : 0;
}


template<typename VarType>
int Grammar<VarType>::FirstIncludeEmpty(ProductionFactory<VarType> *production, size_t child) noexcept {
	if (!generate_first_) {
		if (GenerateFirst() == -1) {
			return -1;
		}
	}
	if (child >= production->size()) {
		return -1;
	}
	
	// result
	bool result = true;
	for (size_t c = child; c < production->size(); ++c) {
		Symbol *symbol = production->Child(c);
		
		if (symbol->Type() > 0) {
			// terminal symbol, first cannot be empty symbol
			result = false;
			break;
		} else if (symbol->Type() == kSymbolType_ProductionFactorySet) {

			// non-terminal symbol, check 
			result = FirstIncludeEmpty(FindSet((ProductionFactorySet<VarType>*)symbol));
			if (!result) break;

		} else {
			// other types, error
			return -1;
		}
	}

	return result ? 1 : 0;
}


template<typename VarType>
int Grammar<VarType>::GenerateFirst() noexcept {

	// init the first_list_
	first_list_.resize(production_sets_.size());
	first_include_empty_.resize(production_sets_.size());


	bool change_first = true;
	while (change_first) {
		change_first = false;


		for (size_t set = 0; set < production_sets_.size(); ++set) {
			for (auto production : *production_sets_[set]) {
// std::cout << "set " << set << " production ?  size " << production->size() << std::endl;

				if (production->size() == 0) {
					// add the empty symbol if meets the empty production
					if (!first_include_empty_[set]) {
						first_include_empty_[set] = true;
						change_first = true;		
					}
				} else {

					for (size_t ic = 0; ic < production->size(); ++ic) {
// std::cout << "child " << ic << std::endl;
						Symbol *child = production->Child(ic);

						if (child->Type() > 0) {

// std::cout << "child type " << child->Type() << std::endl;

							// this child is terminal symbol
							// check existence of this symbol
							auto search = first_list_[set].find(FindSymbol(child));
							if (search == first_list_[set].end()) {			// not exists
// std::cout << "search fail" << std::endl;
								first_list_[set].insert(FindSymbol(child));
								change_first = true;
// std::cout << "after insert, set " << set << "  size " << first_list_[set].size() << std::endl;
							}
							break;


						} else if (child->Type() == kSymbolType_ProductionFactorySet) {

// std::cout << "child type factory set " << std::endl;
							// this child is non-terminal symbol
							
							// add the first of this children symbol to the 
							// first list of its parent
							int child_index = FindSet((ProductionFactorySet<VarType>*)child);
// std::cout << "child index " << child_index << std::endl;
							for (auto first_symbol : first_list_[child_index]) {
// std::cout << "first symbol " << first_symbol << std::endl;

								auto search = first_list_[set].find(first_symbol);

								if (search == first_list_[set].end()) {
// std::cout << "search fail" << std::endl;
									first_list_[set].insert(first_symbol);
									change_first = true;
// std::cout << "after insert, set " << set << "  size " << first_list_[set].size() << std::endl;
								}
							}


							if (!first_include_empty_[child_index]) {
								break;				// break the loop and do not check next child
							}

						} else {


							return -1;
						}
					}
				}
			}
		}


	}

	generate_first_ = true;
	return 0;
}



template<typename VarType>
std::vector<int> Grammar<VarType>::Following(size_t set) noexcept {
	if (!generate_following_) {
		if (GenerateFollowing() == -1) {
			return std::vector<int>(1, -1);
		}
	}
	if (set >= production_sets_.size()) {
		return std::vector<int>(1, -1);
	}

	std::vector<int> result;
	for (int i : following_list_[set]) {
		result.push_back(i);
	}

	return result;
}


template<typename VarType>
int Grammar<VarType>::GenerateFollowing() noexcept {
	following_list_.resize(production_sets_.size());

	// add the fininsh symbol to the start production following list
	following_list_[0].insert(symbol_list_.size());

	bool add_following = true;
	while (add_following) {
		add_following = false;
		for (size_t pset = 0; pset != production_sets_.size(); ++pset) {
			int production_index = 0;
			for (auto production : *(production_sets_[pset])) {

// std::cout << "set " << pset << " production " << production_index << std::endl;

				for (size_t child = 0; child < production->size(); ++child) {
					
// std::cout << "  child " << child << std::endl;

					Symbol *symbol = production->Child(child);
					if (symbol->Type() == kSymbolType_ProductionFactorySet) {


						int set = FindSet((ProductionFactorySet<VarType>*)symbol);

// std::cout << "    type production factory set, index " << set << std::endl;


						if (child == production->size()-1) {

// std::cout << "      last child" << std::endl;
							// add all the Follow symbol of the Parent to the
							// following set of this non-terminal symbol if
							// this is the last child of the production
							for (auto follow : following_list_[pset]) {
								// check its existence
								auto search = following_list_[set].find(follow);
								if (search == following_list_[set].end()) {
									// exists
									following_list_[set].insert(follow);
									add_following = true;
// std::cout << "        add to following list, set " << set << " follow " << follow << std::endl;
								}
							}


						} else {

// std::cout << "      not the last child" << std::endl;

							Symbol *next = production->Child(child+1);
							if (next->Type() > 0) {
								// terminal symbol, its First is itself
// std::cout << "        next child is terminal symbol" << std::endl;

								// check its existence
								int next_index = FindSymbol(next);
								auto search = following_list_[set].find(next_index);

								if (search == following_list_[set].end()) {
									/// exists
									following_list_[set].insert(next_index);
									add_following = true;
// std::cout << "          add to following list, set " << set << "  follow " << next_index << std::endl;

								}
							
							} else if (next->Type() == kSymbolType_ProductionFactorySet) {
								// non-terminal symbol

// std::cout << "        next child is non-terminal symbol" << std::endl;

								// add First symbols of next non-terminal symbol
								// to the following, except for null
								auto first_list = First(production, child+1);
								for (auto next_first : first_list) {

									// check its existence
									auto search = following_list_[set].find(next_first);
									
									if (search == following_list_[set].end()) {
										// exists
										following_list_[set].insert(next_first);
										add_following = true;

// std::cout << "          add to following list, set " << set << "  follow " << next_first << std::endl;
									
									}
								}

								// add Follow of the Parent to the following,
								// if this is the last symbol in a production
								// or its next symbol includes null in First
								if (FirstIncludeEmpty(production, child+1)) {
// std::cout << "          next inclue empty" << std::endl;
									for (auto follow : following_list_[pset]) {
										// check its existence
										auto search = following_list_[set].find(follow);
										if (search == following_list_[set].end()) {
											// exists
											following_list_[set].insert(follow);
											add_following = true;
// std::cout << "            add to following list, set " << set << "  follow " << follow << std::endl;

										}
									}
								
								}

							} else {

								return -1;
						
							}

						}

					} 

				}
				++production_index;
			}
		}
	}



	generate_following_ = true;
	return 0;
}


template<typename VarType>
int Grammar<VarType>::ProductionSetSize() const noexcept {
	return production_sets_.size();
}




//-----------------------------------------------------------------------------
//					explicit instantiations of template classes
//-----------------------------------------------------------------------------

template class Grammar<bool>;
template class Grammar<int>;
template class Grammar<double>;

}	 			// namespace ecl