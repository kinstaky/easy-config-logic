// 配置逻辑使用的语法

#ifndef __LOGIC_DOWNSCALE_GRAMMAR_H__
#define __LOGIC_DOWNSCALE_GRAMMAR_H__

#include "syntax/parser/grammar.h"

namespace ecl {

/**
 * This class represents a logical-downscale grammar used in config logic.
 * And this is the core feature of this project.
 * Action shows the nested downscale layers.
 * 0. S -> L
 * 1. L -> id = E
 * 2. E -> E | T
 * 3. E -> E & T
 * 4. E -> T
 * 5. T -> F / digits
 * 6. T -> F
 * 7. F -> id
 * 8. F -> (E)
 */
class LogicDownscaleGrammar final : public Grammar<int> {
public:

	/// @brief constructor
	/// @exceptsafe Shall not throw exceptions.
	///
	LogicDownscaleGrammar() noexcept;
};


}

#endif // __LOGIC_DOWNSCALE_GRAMMAR_H__
