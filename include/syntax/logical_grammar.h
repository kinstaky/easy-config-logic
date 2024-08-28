/*
 * This file defines the logic grammar class.
 */

#ifndef __LOGICAL_GRAMMAR_H__
#define __LOGICAL_GRAMMAR_H__

#include <string>
#include <vector>

#include "syntax/parser/grammar.h"


namespace ecl {

/**
 * This class represents a logical grammar that only includes operations
 * and '&' and or '|'. The context-free grammar is:
 * 	0. S -> E
 *  1. E -> E | T
 * 	2. E -> E & T
 *  3. E -> T
 *  4. T -> ( E )
 *  5. T -> id
 *
 */
class LogicalGrammar final : public Grammar<bool> {
public:

	/// @brief constructor
	/// @exceptsafe Shall not throw exceptions.
	///
	LogicalGrammar() noexcept;
};

}


#endif /* __LOGIC_GRAMMAR_H__ */