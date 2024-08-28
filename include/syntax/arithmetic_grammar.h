#ifndef __ARITHMETIC_GRAMMAR_H__
#define __ARITHMETIC_GRAMMAR_H__

#include <vector>

#include "syntax/parser/production.h"
#include "syntax/parser/token.h"
#include "syntax/parser/grammar.h"

namespace ecl {

/**
 * This class represents a arithmetic grammar only includes the operation
 * addition '+' and multiplication '*'.
 *
 */
class AddMultiGrammar final : public Grammar<double> {
public:

	/// @brief constructor
	/// @exceptsafe Shall not throw exceptions.
	///
	AddMultiGrammar() noexcept;
};



/**
 * This class represents a arithmetic grammar includes the operation
 * addition '+', subtraction '-', multiplication '*', and division '/'.
 *
 */
class ArithmeticGrammar final : public Grammar<double> {
public:

	/// @brief constructor
	/// @exceptsafe Shall not throw exceptions.
	///
	ArithmeticGrammar() noexcept;
};

}			// namespace ecl

#endif /* __ARITHMETIC_GRAMMAR_H__ */