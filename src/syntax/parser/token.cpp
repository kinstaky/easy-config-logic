#include "syntax/parser/token.h"

#include <string>

namespace ecc {


//-----------------------------------------------------------------------------
// 						Symbol
//-----------------------------------------------------------------------------

Symbol::Symbol(int type) noexcept
:type_(type) {
}




//-----------------------------------------------------------------------------
// 						Token
//-----------------------------------------------------------------------------

Token::Token(int type, const std::string &value) noexcept
:Symbol(type), value_(value) {
}



//-----------------------------------------------------------------------------
// 					Identifier
//-----------------------------------------------------------------------------

Identifier::Identifier(const std::string &value) noexcept
:Token(kSymbolType_Identifier, value) {
}


Identifier::Identifier(char value) noexcept
:Identifier(std::string(1, value)) {
}


int Identifier::Attach(void *var_ptr) noexcept {
	if (!var_ptr) return -1;
	attached_ = var_ptr;
	return 0;
}


void* Identifier::GetAttached() const noexcept {
	return attached_;
}


//-----------------------------------------------------------------------------
// 					Operator
//-----------------------------------------------------------------------------

Operator::Operator(const std::string &value) noexcept
:Token(kSymbolType_Operator, value) {
}


Operator::Operator(char value) noexcept
:Operator(std::string(1, value)) {
}

}				// namespace ecc