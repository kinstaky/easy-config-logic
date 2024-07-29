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

Token::Token(int type, const std::string &name) noexcept
: Symbol(type), name_(name) {
}



//-----------------------------------------------------------------------------
// 									Variable
//-----------------------------------------------------------------------------

Variable::Variable(const std::string &name) noexcept
: Token(kSymbolType_Variable, name) {
}


Variable::Variable(char name) noexcept
: Variable(std::string(1, name)) {
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

Operator::Operator(const std::string &name) noexcept
:Token(kSymbolType_Operator, name) {
}


Operator::Operator(char name) noexcept
:Operator(std::string(1, name)) {
}


//-----------------------------------------------------------------------------
//								NumberLiteral
//-----------------------------------------------------------------------------

NumberLiteral::NumberLiteral(int value) noexcept
: Token(kSymbolType_Literal, std::to_string(value))
, value_(value) {

}

}				// namespace ecl