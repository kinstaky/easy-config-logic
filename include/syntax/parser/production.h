/*
 * This file is part of the context-free grammar library.
 *
 */

#ifndef __PRODUCTION_H__
#define __PRODUCTION_H__

#include <exception>
#include <stdexcept>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <vector>

#include "syntax/parser/token.h"

namespace ecl {

template<typename EvalType>
using ActionType = std::function<EvalType(const std::vector<Symbol*>&)>;


/**
 * This class is the base class of Prouction, ProductionFactory and
 * ProductionFactoryItem classed. It implements the SetChild and SetChildren
 * methods.
 */
class ProductionBase : public Symbol {
public:

	/// @brief constructor
	/// @note Put this constructor in protected field to prevent its creation.
	/// 	So only the creation of inherited class is allowed.
	///
	/// @param[in] parent pointer to the parent
	/// @param[in] size number of symbols in right side of the production
	/// @param[in] type type of the symbol, kSymbolType_Production,
	/// 	kSymbolType_ProductionFactory or kSymbolType_ProductionItem
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	ProductionBase(Symbol *parent, size_t size, int type) noexcept;



	/// @brief default destructor
	///
	virtual ~ProductionBase() = default;


	/// @brief set parent
	///
	/// @param[in] parent pointer to new parent
	/// @returns 0 on success, -1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline int SetParent(ProductionBase *parent) noexcept {
		parent_ = parent;
		return 0;
	}


	/// @brief abstract method return the parent
	///
	/// @returns the pointer to the parent object
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline Symbol* Parent() const noexcept {
		return parent_;
	}
 

	/// @brief get the size of children (the left hand side symbols)
	///
	/// @returns size of children (the left hand side symbols)
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline size_t size() const noexcept {
		return children_.size();
	}


	/// @brief set child of the syntax tree
	/// @note This method set child of the vector symbols,
	/// 	.i.e. the syntax tree.
	///
	/// @param[in] index index of the child node in the vector
	/// @param[in] child pointer of the symbol(production or token)
	/// @returns 0 if success, -1 on index out of bounds, -2 on invalid 
	///		symbol pointer
	///
	/// @exceptsafe shall not throw exceptions.
	///
	int SetChild(size_t index, Symbol *child) noexcept;


	/// @brief set the last child of production
	/// @note This is the terminal function of the recursively SetChildren
	///		method.
	///
	/// @param[in] child pointer to the child being set
	/// @returns 0 on success, -1 on index out of bounds
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int SetChildren(Symbol *child) noexcept;


	/// @brief recursively set children of production
	/// @note This is a recursive function and set children except for the
	/// 	last one.
	/// 
	/// @tparam ArgTypes type of the children (should be Symbol or inherit
	///		from Symbol)
	/// @param[in] first pointer to the first setting child
	/// @param[in] children pointer to the children
	/// @returns 0 on success, -1 on index out of bounds
	///
	/// @overload
	///
	///	@exceptsafe Shall not throw exceptions.
	///
	template<typename... ArgTypes>
	int SetChildren(Symbol *first, ArgTypes... children) noexcept {
		// first child
		if (setting_child_ == -1) {
			for (size_t i = 0; i != children_size_; ++i) {
				children_[i] = nullptr;
			}
			setting_child_ = 0;
		}

		// set child
		if (setting_child_ >= int(children_size_)) {
			return -1;
		}
		children_[setting_child_] = first;

		// recursively set children
		++setting_child_;
		return SetChildren(children...);
	}


	/// @brief get child from production through index
	///
	/// @param[in] index index of child to get
	/// @returns pointer to the child at index
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline Symbol* Child(size_t index) const noexcept {
		return children_[index];
	}


	/// @brief check this production is complete
	///
	/// @returns true on complete, otherwise false
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline bool IsComplete() const noexcept {
		for (auto child : children_) {
			if (!child) return false;
		}
		return true;
	}


	

protected:

	Symbol *parent_;

	size_t children_size_;		// size of the symbols on the right side of production
	std::vector<Symbol*> children_;				// symbols of this production

private:
	int setting_child_;							// flag showing setting children
};




/**
 * Production class represents the concrete syntax in the grammar. It contains
 * the concrete syntax tree, the value of the production can be evaluated
 * according to the action function.
 */
template<typename EvalType>
class Production final : public ProductionBase {
public:

	/// @brief constructor
	///
	/// @param[in] parent pointer to the parent production
	/// @param[in] size size of the symbols on the right side of production
	/// @param[in] action pointer to the action function, get from factory.  
	/// @param[in] origin pointer to the origin production factory
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	Production(
		Symbol *parent, 
		size_t size, 
		ActionType<EvalType> *action, 
		Symbol *origin = nullptr
	) noexcept;


	/// @brief default destructor
	///
	~Production() = default;


	/// @brief evaluate the value of production
	/// @note This method evaluates the value of this production. It
	/// 	recursively evaluates the value of children nodes and combines
	/// 	them according to the action.
	///
	/// @returns value of the production
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline EvalType Eval() const noexcept {
		if (action_) {
			return (*action_)(children_);
		} else {
			return static_cast<EvalType>(0);
		}
	}


	/// @brief get the origin production factory pointer
	///
	/// @returns the pointer to the origin production factory
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline Symbol* Origin() const noexcept {
		return origin_;
	}




private:
	ActionType<EvalType> *action_;				// action when evaluating
	Symbol *origin_;		// origin of the production
};



template<typename EvalType>
class ProductionItem;

/**
 * This class applys the factory pattern and creates the Production instance. 
 * This ProductionFactory class represents the abstract syntax in the grammar.
 */
template<typename EvalType>
class ProductionFactory final : public ProductionBase {
public:


	///@brief constructor
	///
	/// @param[in] size size of the symbols on the right side of production
	/// @param[in] action pointer to the action function, default is nullptr  
	///
	/// @overload
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	ProductionFactory(Symbol *parent, size_t size, const ActionType<EvalType> &action = nullptr) noexcept;



	/// @brief destructor
	///
	virtual ~ProductionFactory() noexcept;



	/// @brief create a Production instance
	///
	/// @param[in] symbols stack of symbols to create the concrete production
	/// @returns pointer to the production instance, nullptr on error
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	Production<EvalType>* CreateProduction(std::stack<Symbol*> symbols) noexcept;



	/// @brief recursively generate items
	/// @note Item represents the status of a production in the LR syntax parse
	/// 	process. For example, the item "E -> E '.' + T" of the production
	/// 	"E -> E + T" means the item have received the production E and
	/// 	expects a symbol '+' followed by production T. The dot '.' shows
	/// 	the status.
	///
	/// @returns pointer to the next item, or nullptr if finish
	///
	/// @exceptsafe Shall not throw exceptions.
	ProductionItem<EvalType>* GenerateItems() noexcept;


	/// @brief get the nth item
	///
	/// @param[in] index index of the item
	/// @returns pointer to the specified item, or nullptr if out of bounds
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	ProductionItem<EvalType> *Item(size_t index) noexcept;

private:
	ActionType<EvalType> action_;				// action when evaluating

	size_t generating_item_;						// the next item index to generate
	std::vector<ProductionItem<EvalType>*> items_;	// the generated items
};



/**
 * ProductionFactorySet represents a set of production that deduced from the
 * same nonterminal symbol and seperated by '|' in the context-free grammar.
 * For example, in the grammar below:
 * 	E -> E + T | T
 * E -> E + T is representey by a production factory and E -> T is
 * represented by another. They belongs to one production factory set.
 */
template<typename EvalType>
class ProductionFactorySet final : public Symbol {
public:

	/// @brief constructor
	///
	ProductionFactorySet();

	/// @brief default destructor
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	~ProductionFactorySet() = default;

	/// @brief add production factory
	/// 
	/// @param[in] production the added production
	/// @returns  0 on success
	/// 
	/// @exceptsafe Shall not throw exceptions.
	///
	int AddProductionFactory(ProductionFactory<EvalType>* production) noexcept;


	/// @brief returns the production at the position index
	///
	/// @param[in] index index of the production
	/// @returns the production at the position index
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline ProductionFactory<EvalType>* operator[](size_t index) const noexcept {
		return productions_[index];
	}


	/// @brief check whether the production factory in this set are complete
	///
	/// @returns true if all the production factories are complete, otherwise false
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline bool IsComplete() const noexcept {
		for (auto p : productions_) {
			if (!p->IsComplete()) return false;
		}
		return true;
	}


	/// @brief get frist element of the iterator
	///
	/// @returns first element of the iterator
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline auto begin() noexcept {
		return productions_.begin();
	}
	
	
	/// @brief get the past-the-end element of the production iterator
	///
	/// @returns the past-the-end element of the production iterator
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline auto end() noexcept {
		return productions_.end();
	}


	/// @brief get the production size
	///
	/// @returns size of the production
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline auto size() const noexcept {
		return productions_.size();
	}
	

private:
	std::vector<ProductionFactory<EvalType>*> productions_;
};



/**
 * ProductionItem is the item of a production that used in the LR syntax parser.
 * It represents one of the status of a production in the reduced process.
 */
template<typename EvalType>
class ProductionItem final : public ProductionBase {
public:
	
	/// @brief constructor
	///
	/// @param[in] parent left hand side of production, i.e. ProductionFactorySet
	/// @param[in] size size of the symbols on the right side of production
	/// @param[in] index index of this item 
	/// @param[in] origin origin of the item, i.e. the ProductionFactory
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	ProductionItem(
		Symbol *parent,
		size_t size,
		size_t index,
		ProductionFactory<EvalType> *origin
	) noexcept;


	/// @brief default destructor
	///
	virtual ~ProductionItem() = default;


	/// @brief get the origin of this item
	/// @note This method gets the pointer orgin of this item, i.e. the
	/// 	ProductionFactory generates	this ProductionItem.
	///
	/// @returns the origin of the item
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline ProductionFactory<EvalType>* Origin() const noexcept {
		return origin_;
	}


	// /// @brief get the parent of the item
	// /// @note This method gets the pointer to the parent of this item,
	// /// 	i.e. the ProductionFactorySet on the left hand side of the
	// ///		production.
	// ///
	// /// @returns the parent of the item
	// /// 
	// /// @exceptsafe Shall not throw exceptions.
	// ///
	// inline ProductionFactorySet<EvalType>* Parent() const noexcept;



	/// @brief get the expected symbol
	///
	/// @returns the expected symbol, nullptr if this is the last item.
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline Symbol* ExpectedSymbol() const noexcept {
		return IsLast() ? nullptr : children_[item_index_];
	}


	/// @brief get whether this item is the last item
	///
	/// @returns true if this item is the last item, false otherwise
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline bool IsLast() const noexcept {
		return item_index_ == children_size_;
	}


	/// @brief get the item index
	///
	/// @returns the item index
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline size_t Index() const noexcept {
		return item_index_;
	}

	
private:
	size_t item_index_;								// index of this item

	// The ProductionFactory generates this item. 
	ProductionFactory<EvalType> *origin_;
};



/**
 * ProductionItemCollection is the collection of the production items.
 */
template<typename EvalType>
class ProductionItemCollection {
public:
	
	/// @brief constructor
	///
	ProductionItemCollection() = default;


	/// @brief default destructor
	///
	~ProductionItemCollection() = default;


	/// @brief add item to this collection
	///
	/// @param[in] item added item
	/// @param[in] core whether is core item
	/// @returns 0 on success, -1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int AddItem(ProductionItem<EvalType> *item, bool core) noexcept;
	

	/// @brief goto function of collections meets symbol
	///
	/// @param[in] symbol the next symbol
	/// @returns pointer to the next item collection, nullptr if no destination
	///
	/// @exceptsafe shall not throw exceptions
	///
	ProductionItemCollection<EvalType>* Goto(Symbol *symbol) const noexcept;


	/// @brief set goto destination
	///
	/// @param[in] symbol pointer to the next symbol
	/// @param[in] collection destination collection
	/// returns 0 on success, -1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline int SetGoto(Symbol *symbol, ProductionItemCollection<EvalType> *collection) noexcept {
		const auto result = goto_table_.insert(std::make_pair(symbol, collection));
		return result.second ? 0 : -1;
	}


	/// @brief get the first element of the collection iterator
	///
	/// @returns first element of the iterator, represents the first item
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline auto begin() noexcept {
		return items_.begin();
	}
	
	
	/// @brief get the past-the-end element of the production iterator
	///
	/// @returns the past-the-end element of the production iterator
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline auto end() noexcept {
		return items_.end();
	}


	/// @brief get the production size
	///
	/// @returns size of the production
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline auto size() const noexcept {
		return items_.size();
	}


private:
	std::vector<ProductionItem<EvalType>*> items_;
	std::vector<ProductionItem<EvalType>*> core_items_;
	std::vector<ProductionItem<EvalType>*> non_core_items_;

	std::map<Symbol*, ProductionItemCollection<EvalType>*> goto_table_;
};




/// @brief helper function to evaluate symbol
/// @note This method helps generate the action function, since
/// 	it handles the evaluation of all kinds of symbols.
/// @relatesalso Production 
///
/// @tparam EvalType type of the expression evaluated
/// @param[in] symbol pointer to the symbol being evaluated
///
/// @returns evaluation value
///
/// @exception runtime_error invalid symbol type or not evaluable symbol type
///
template<typename EvalType>
EvalType Evaluate(Symbol *symbol) {
	EvalType result;
	switch (symbol->Type()) {
		case kSymbolType_Production:
			result = ((Production<EvalType>*)symbol)->Eval();
			break;
		case kSymbolType_Identifier:
			result = *(static_cast<EvalType*>(((Identifier*)symbol)->GetAttached()));
			break;
		case kSymbolType_Operator:
			throw std::runtime_error("operator symbol");
		case kSymbolType_ProductionFactory:
			throw std::runtime_error("production factory symbol");
		case kSymbolType_ProductionFactorySet:
			throw std::runtime_error("production factory set symbol");
		default:
			throw std::runtime_error("invalid symbol type");
	}
	return result;
}


}				// namespace ecl

#endif /* __PRODUCTION_H__ */
