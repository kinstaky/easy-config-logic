#include "syntax/parser/token.h"

#include <string>

namespace ecl {


//-----------------------------------------------------------------------------
// 									Symbol
//-----------------------------------------------------------------------------

Symbol::Symbol(int type) noexcept
: type_(type) {
}




//-----------------------------------------------------------------------------
// 									Token
//-----------------------------------------------------------------------------

Token::Token(
	int type,
	const std::string &name,
	size_t position,
	size_t size
) noexcept
: Symbol(type), name_(name), position_(position), size_(size) {
}



//-----------------------------------------------------------------------------
// 									Variable
//-----------------------------------------------------------------------------

Variable::Variable(
	const std::string &name,
	size_t position,
	size_t size
) noexcept
: Token(kSymbolType_Variable, name, position, size) {
}


Variable::Variable(char name, size_t position, size_t size) noexcept
: Variable(std::string(1, name), position, size) {
}


int Variable::Attach(void *var_ptr) noexcept {
	if (!var_ptr) return -1;
	attached_ = var_ptr;
	return 0;
}


void* Variable::GetAttached() const noexcept {
	return attached_;
}


//-----------------------------------------------------------------------------
// 										Operator
//-----------------------------------------------------------------------------

Operator::Operator(const std::string &name, size_t position) noexcept
:Token(kSymbolType_Operator, name, position, name.size()) {
}


Operator::Operator(char name, size_t position) noexcept
:Token(kSymbolType_Operator, std::string(1, name), position, 1) {
}


//-----------------------------------------------------------------------------
//								NumberLiteral
//-----------------------------------------------------------------------------

NumberLiteral::NumberLiteral(int value, size_t position, size_t size) noexcept
: Token(kSymbolType_Literal, std::to_string(value), position, size)
, value_(value) {
}

}				// namespace ecl