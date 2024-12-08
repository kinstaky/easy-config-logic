#ifndef __PARSE_RESULT_H__
#define __PARSE_RESULT_H__

#include <string>

namespace ecl {

/*
 * status
 *   1 Invalid character
 *   2 Variable starts with digits
 *   3 Variable starts with underscore
 * 101 Invalid token
 * 102 Token can't be shifted
 * 103 Invalid token type
 * 104 Invalid action type
 * 201 Tokens less than 3
 * 202 Invalid token type
 * 203 Multiple source of output
 * 204 Input and output in same port
 * 205 Invalid scaler input
 * 206 LEMO and LVDS in same port
 * 207 Undefined variable
 * 208 Nested downscale expression
 * 209 Invalid external clock source
 * 300 Generate error
 */
class ParseResult {
public:

	/// @brief constructor
	/// @param[in] status status of result, 0 is success
	/// @param[in] position position of error in expression
	/// @param[in] length length of error string
	///
	ParseResult(
		int status,
		size_t position = 0,
		size_t length = 1
	);


	/// @brief status is ok
	/// @returns true if sucessful, false otherwise
	///
	inline bool Ok() const {
		return status_ == 0;
	}


	/// @brief get parse status
	/// @returns parse status
	///
	inline int Status() const {
		return status_;
	}


	/// @brief get error position
	/// @returns error position
	///
	inline size_t Position() const {
		return position_;
	}


	/// @brief get error token length
	/// @returns length of error token
	///
	inline size_t Length() const {
        return length_;
    }


	/// @brief get error message
	/// @param[in] line expression
	/// @returns error message
	///
	std::string Message(const std::string &line) const;


private:
	int status_;
	size_t position_;
	size_t length_;
};

};	// ecl

#endif	// __PARSE_RESULT_H__