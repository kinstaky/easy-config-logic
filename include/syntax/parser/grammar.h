/*
 * This file is part of the context-free grammar library.
 */


#ifndef __GRAMMAR_H__
#define __GRAMMAR_H__

#include <map>
#include <set>
#include <string>
#include <vector>

#include "syntax/parser/production.h"
#include "syntax/parser/token.h"

namespace ecl {

template<typename VarType>
class Grammar {
public:

	/// @brief constructor
	///
	Grammar() noexcept;

	/// @brief destructor
	///
	virtual ~Grammar() noexcept;

	/// @brief add production set to the grammar
	///
	/// @param[in] set the set added to the grammar
	/// @param[in] start this is the start set of the grammar
 	/// @returns 0 on success, -1 on failure
	///
	///	@exceptsafe Shall not throw exceptions.
	///
	int AddProductionSet(ProductionFactorySet<VarType> *set, bool start = false) noexcept;



	/// @brief check the completion of the grammar
	///
	/// @returns true if this is complete, false incomplete
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	bool IsComplete() noexcept;


	/// @brief get the symbol list of this grammar
	///
	/// @returns the symbol list
	///
	/// @exceptsafe Shall not throw exceptions.
	/// 
	std::vector<Symbol*> SymbolList() const noexcept;



	/// @brief generate the item collection of productions
	///
	/// @param[in] look_ahead size of looking ahead characters
	/// @returns size of collections on success, -1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int GenerateCollections(int look_ahead = 0) noexcept;


	/// @brief get the collection through index
	///
	/// @param[in] index index of the collection
	/// @returns the pointer to the specified collection, nullptr on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	inline ProductionItemCollection<VarType>* Collection(size_t index) noexcept {
		if (index >= collections_.size()) {
			return nullptr;
		}
		return collections_[index];
	}



	/// @brief generate the non-core items in the core item closure
	///
	/// @param[in] item item that generate the closure for
	/// @param[out] closure closure to be generated
	/// @returns 0 on success, -1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int MakeClosure(ProductionItem<VarType> *item, std::vector<ProductionItem<VarType>*>  &closure) noexcept;


	/// @brief get the next item which receive one more symbol
	/// @note This function returns the next item and the moving symbol.
	/// 	For example, the input itme is "E -> E '.' + T". And here, '.'
	/// 	represents for the middle point of the item. It means that this
	/// 	item have received the production E, and expects the next symbol
	/// 	is '+' followed by production T. So the returnning next item will
	///		be "E -> E + '.' T" and the next symbol will be '+'.
	///
	/// @param[in] item this item
	/// @param[out] symbol the expected symbol of this item
	/// @returns the next item with one more symbol
	/// 
	/// @exceptsafe Shall not throw exceptions.
	///
	ProductionItem<VarType>* NextItem(ProductionItem<VarType> *item, Symbol **symbol) noexcept;
	


	/// @brief get the goto action through the collection and symbol index
	///
	/// @param[in] collection the collection index
	/// @param[in] symbol the symbol index
	/// @returns the collection index to go, or -1 on error
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int CollectionGoto(size_t collection, size_t symbol) noexcept;


	/// @brief find the possible first symbols of a non-terminal item
	///
	/// @param[in] set the index of the production set
	/// @returns a set of symbols of possible first symbols
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	std::vector<int> First(size_t set) noexcept;


	/// @brief find the possible first symbols for a list of symbols
	/// @note This function finds the possible first symbols for a list of
	/// 	symbols and the list is part of the right-hand side of a
	/// 	production. So the list can be represented by a production
	///		and the index of child where ths list begins.
	///
	/// @param[in] production the production which contains the list
	/// @param[in] child the start index of the list
	/// @returns a set of symbol of possible first symbols
	///
	/// @overload
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	std::vector<int> First(ProductionFactory<VarType> *production, size_t child) noexcept;



	/// @brief whether a set contains null production as first symbol
	///
	/// @param[in] set the index of the production set
	/// @returns 1 for true, 0 for false, -1 for error
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int FirstIncludeEmpty(size_t set) noexcept;



	/// @brief whether a list of symbols contains null symbol as first symbol
	/// @note This function checks a list of symbols containing of null symbol
	///		and the list is part of the right-hand side of a production. So
	/// 	the list can be represented by a production and the index of child
	/// 	where ths list begins.
	///
	/// @param[in] production the production which contains the list
	/// @param[in] child the start index of the list
	/// @returns 1 for true, 0 for false, -1 for error
	///
	/// @overload
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int FirstIncludeEmpty(ProductionFactory<VarType> *production, size_t child) noexcept;



	/// @brief gernerate the first list and first include empty vector
	///
	/// @returns 0 on success, 1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int GenerateFirst() noexcept;



	/// @brief find the follwing symbol of a non-terminal symbol
	///
	/// @param[in] set the index of the production set
	/// @returns vector of the follwing symbol index
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	std::vector<int> Following(size_t set) noexcept;


	/// @brief generate the following table
	///
	/// @returns 0 on success, -1 on failure
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int GenerateFollowing() noexcept;


	/// @brief get the index of the production set
	///
	/// @param[in] set the finding production set
	/// @returns the index of the production set, -1 on error
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int FindSet(ProductionFactorySet<VarType> *set) noexcept;


	/// @brief get the index of the symbol
	///
	/// @param[in] symbol finding symbol, includes production set and token
	/// @returns the index of the symbol, -1 on error
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int FindSymbol(Symbol *symbol) noexcept;


	/// @brief get the size of the production set
	///
	/// @returns the size of the production set, or -1 on error
	///
	/// @exceptsafe Shall not throw exceptions.
	///
	int ProductionSetSize() const noexcept;


	/// @brief get the specific production set through index
	///
	/// @param[in] index index of the production set
	/// @returns the specific production set, or nullptr for error
	///
	/// exceptsafe Shall not throw exceptions.
	///
	inline ProductionFactorySet<VarType>* ProductionSet(size_t index) const noexcept {
		if (index < 0 || index >= production_sets_.size()) {
			return nullptr;
		}
		return production_sets_[index];
	}

protected:


	std::vector<ProductionFactorySet<VarType>*> production_sets_;
	std::vector<Symbol*> symbol_list_;

	bool generate_first_;				// whether has generated first list
	std::vector<std::set<int>> first_list_;
	std::vector<bool> first_include_empty_;


	bool generate_following_;					// show whether has generated following list
	std::vector<std::set<int>> following_list_;




	std::vector<ProductionItem<VarType>*> core_items_;		// generated core items
	std::vector<ProductionItem<VarType>*> non_core_items_; 	// generated non core items
	std::vector<ProductionItemCollection<VarType>*> collections_;	// collection of items

};

}				// namespace ecl

#endif /* __GRAMMAR_H__ */