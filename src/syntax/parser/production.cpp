#include "syntax/parser/production.h"

#include <exception>
#include <set>
#include <stack>

#include "syntax/parser/token.h"


namespace ecc {

//-----------------------------------------------------------------------------
// 									ProductionBase
//-----------------------------------------------------------------------------

ProductionBase::ProductionBase(Symbol *parent, size_t size, int type) noexcept
: Symbol(type), parent_(parent), children_size_(size), setting_child_(-1) {
	children_.resize(size);
	for (size_t i = 0; i < size; ++i) {
		children_[i] = nullptr;
	}
}



int ProductionBase::SetChild(size_t index, Symbol *child) noexcept {
	if (index >= children_size_) return -1;
	if (!child) return -2;
	children_[index] = child;
	return 0;
}



int ProductionBase::SetChildren(Symbol *child) noexcept {
	// first child
	if (setting_child_ == -1) {
		for (size_t i = 0; i != children_size_; ++i) {
			children_[i] = nullptr;
		}
		setting_child_ = 0;
	}

	// not the last child, error
	if (setting_child_ != int(children_size_ - 1)) {
		return -1;
	}

	// set child
	children_[setting_child_] = child;

	// last one, finish the recursive SetChildren
	setting_child_ = -1;
	return 0;
}


// template<typename... ArgTypes>
// int ProductionBase::SetChildren(Symbol *first, ArgTypes... children) noexcept {
// 	// first child
// 	if (setting_child_ == -1) {
// 		for (int i = 0; i != children_size_; ++i) {
// 			children_[i] = nullptr;
// 		}
// 		setting_child_ = 0;
// 	}

// 	// set child
// 	if (setting_child_ >= children_size_) {
// 		return -1;
// 	}
// 	children_[setting_child_] = first;

// 	// recursively set children
// 	++setting_child_;
// 	return SetChildren(children...);
// }



//-----------------------------------------------------------------------------
// 									Production
//-----------------------------------------------------------------------------

template<typename EvalType>
Production<EvalType>::Production(
	Symbol *parent, 
	size_t size, 
	ActionType<EvalType> *action, 
	Symbol *origin
) noexcept
:ProductionBase(parent, size, kSymbolType_Production), action_(action), origin_(origin) {
}


// template<typename EvalType>
// EvalType Production<EvalType>::Eval() const noexcept {
// 	return (*action)(symbols);
// }



//-----------------------------------------------------------------------------
// 								ProductionFactory
//-----------------------------------------------------------------------------

template<typename EvalType>
ProductionFactory<EvalType>::ProductionFactory(Symbol *parent, size_t size, const ActionType<EvalType> &action) noexcept
:ProductionBase(parent, size, kSymbolType_ProductionFactory), action_(action) {
	children_.resize(size);
	for (size_t i = 0; i != size; ++i) {
		children_[i] = nullptr;
	}
	generating_item_ = 0;
}



template<typename EvalType>
ProductionFactory<EvalType>::~ProductionFactory() noexcept {
	for (ProductionItem<EvalType>* item : items_) {
		delete item;
	}
}



template<typename EvalType>
ProductionItem<EvalType>* ProductionFactory<EvalType>::GenerateItems() noexcept {
	// out of bound, we have finished generating items
	if (generating_item_ > children_size_) {
		generating_item_ = 0;
		return nullptr;
	}
	// return the existing items
	if (generating_item_ < items_.size()) {
		return items_[generating_item_++];
	}
	// generate the new items
	ProductionItem<EvalType> *new_item = new ProductionItem<EvalType>(parent_, children_size_, generating_item_, this);
	for (size_t i = 0; i != children_size_; ++i) {
		new_item->SetChild(i, children_[i]);
	}
	items_.push_back(new_item);

	++generating_item_;
	return new_item;
}



template<typename EvalType>
Production<EvalType>* ProductionFactory<EvalType>::CreateProduction(std::stack<Symbol*> symbols) noexcept {
	Production<EvalType>* result = new Production<EvalType>(nullptr, children_size_, &action_, this);
	for (int i = children_.size()-1; i >=0; --i) {
		Symbol *symbol = symbols.top();
		symbols.pop();
		result->SetChild(i, symbol);
		if (symbol->Type() == kSymbolType_Production) {
			((Production<EvalType>*)symbol)->SetParent(result);
		}
	}
	return result;
}



template<typename EvalType>
ProductionItem<EvalType>* ProductionFactory<EvalType>::Item(size_t index) noexcept {
	if (items_.empty()) {
		while (GenerateItems());
	}
	if (index >= items_.size()) return nullptr;
	return items_[index];
}




//-----------------------------------------------------------------------------
// 							ProductionFactorySet
//-----------------------------------------------------------------------------

template<typename EvalType>
ProductionFactorySet<EvalType>::ProductionFactorySet()
:Symbol(kSymbolType_ProductionFactorySet) {
}


template<typename EvalType>
int ProductionFactorySet<EvalType>::AddProductionFactory(ProductionFactory<EvalType> *production) noexcept {
	productions_.push_back(production);
	return 0;
}


//-----------------------------------------------------------------------------
//								ProductionItem
//-----------------------------------------------------------------------------

template<typename EvalType>
ProductionItem<EvalType>::ProductionItem(Symbol *parent, size_t size, size_t index, ProductionFactory<EvalType>* origin) noexcept
:ProductionBase(parent, size, kSymbolType_ProductionItem), item_index_(index), origin_(origin) {
}


// template<typename EvalType>
// ProductionFactorySet<EvalType>* ProductionItem<EvalType>::Parent() const noexcept {
// 	return parent_;
// }



//-----------------------------------------------------------------------------
//								ProductionItemCollection
//-----------------------------------------------------------------------------

template<typename EvalType>
ProductionItemCollection<EvalType>* ProductionItemCollection<EvalType>::Goto(Symbol* symbol) const noexcept {
	auto search = goto_table_.find(symbol);
	if (search != goto_table_.end()) {
		return search->second;
	}
	return nullptr;
}



template<typename EvalType>
int ProductionItemCollection<EvalType>::AddItem(ProductionItem<EvalType>* item, bool core) noexcept {
	if (core) {
		core_items_.push_back(item);
	}
	else {
		non_core_items_.push_back(item);
	}
	items_.push_back(item);
	return 0;
}



//-----------------------------------------------------------------------------
// 									Helper functions
//-----------------------------------------------------------------------------


// template<typename EvalType>
// EvalType Evaluate(Symbol *symbol) {
// 	EvalType result;
// 	switch (symbol->Type()) {
// 		case kSymbolType_Production:
// 			result = ((Production<EvalType>*)symbol)->Eval();
// 			break;
// 		case kSymbolType_Identifier:
// 			// result = *(static_cast<Identifier*>(Symbol)->GetAttached()));
// 			break;
// 		case kSymbolType_Operator:
// 			throw std::runtime_error("operator symbol");
// 		case kSymbolType_ProductionFactory:
// 			throw std::runtime_error("production factory symbol");
// 		case kSymbolType_ProductionFactorySet:
// 			throw std::runtime_error("production factory set symbol");
// 		default:
// 			throw std::runtime_error("invalid symbol type");
// 	}
// 	return result;
// }




//-----------------------------------------------------------------------------
//					explicit instantiations of template classes
//-----------------------------------------------------------------------------

// production
template class Production<bool>;
template class Production<int>;
template class Production<double>;

// production factory
template class ProductionFactory<bool>;
template class ProductionFactory<int>;
template class ProductionFactory<double>;

// production factory set
template class ProductionFactorySet<bool>;
template class ProductionFactorySet<int>;
template class ProductionFactorySet<double>;


// production items
template class ProductionItem<bool>;
template class ProductionItem<int>;
template class ProductionItem<double>;


// production item collection
template class ProductionItemCollection<bool>;
template class ProductionItemCollection<int>;
template class ProductionItemCollection<double>;

}				// namespace ecc