#include "parse_result.h"

#include <sstream>

namespace ecl {

ParseResult::ParseResult(
	int status,
	size_t position,
	size_t length
)
: status_(status)
, position_(position)
, length_(length) {

}


std::string ErrorWord(const std::string &line, size_t pos, size_t length) {
	std::stringstream ss;
	ss << "  " << line.substr(0, pos)
		<< "\033[31m" << line.substr(pos, length) << "\033[0m"
		<< line.substr(pos+length) << "\n";
	return ss.str();
}


std::string ErrorCharacter(const std::string &line, size_t pos) {
	return ErrorWord(line, pos, 1);
}

std::string ParseResult::Message(const std::string &line) const {
	if (Ok()) return "";
	std::stringstream ss;
	ss << "[Error] ";
	if (status_ ==  1) {
		ss << "Invalid character " << line[position_]
			<< " at " << position_ << "\n"
			<< ErrorCharacter(line, position_);
	} else if (status_ == 2) {
		ss << "Variable can't be start with digits, position "
			<< position_ << "\n"
			<< ErrorCharacter(line, position_);
	} else if (status_ == 3) {
		ss << "Variable can't be start with underscore '_', position "
			<< position_ << "\n"
			<< ErrorCharacter(line, position_);
	} else if (status_ == 101) {
		ss << "Syntax error, invalid token: "
			<< line.substr(position_, length_) << "\n"
			<< ErrorWord(line, position_, length_);
	} else if (status_ == 102) {
		ss << "Syntax error, this token can't be shifted: "
			<< line.substr(position_, length_) << "\n"
			<< ErrorWord(line, position_, length_);
	} else if (status_ == 103) {
		ss << "Syntax error, invalid token type: "
			<< line.substr(position_, length_) << "\n"
			<< ErrorWord(line, position_, length_);
	} else if (status_ == 104) {
		ss << "Syntax error, invalid action type when looking: ";
		if (position_ == line.length()) {
			ss << "&.\n"
				<< ErrorWord((line+"&"), position_, length_);
		} else {
			ss << line.substr(position_, length_) << "\n"
				<< ErrorWord(line, position_, length_);
		}
	} else if (status_ == 201) {
		ss << "Size of token is less than 3.\n"
			<< line << "\n";
	} else if (status_ == 202) {
		ss << "Invalid type of token: "
			<< line.substr(position_, length_) << "\n"
			<< ErrorWord(line, position_, length_);
	} else if (status_ == 203) {
		ss << "Multiple source of output: "
			<< line.substr(position_, length_) << "\n"
			<< ErrorWord(line, position_, length_);
	} else if (status_ == 204) {
		ss << "Input and output in the same port: "
			<< line.substr(position_, length_) << "\n"
			<< ErrorWord(line, position_, length_);
	} else if (status_ == 205) {
		ss << "Invalid scaler input: "
			<< line.substr(position_, length_) << "\n"
			<< ErrorWord(line, position_, length_);
	} else if (status_ == 206) {
		ss << "Port defined as LEMO and LVDS at the same time: "
			<< line.substr(position_, length_) << "\n"
			<< ErrorWord(line, position_, length_);
	} else if (status_ == 207) {
		ss << "Undefined variable: "
			<< line.substr(position_, length_) << "\n"
			<< ErrorWord(line, position_, length_);
	} else if (status_ == 208) {
		ss << "Unable to parse nested downscale expression.\n";
	} else if (status_ == 209) {
		ss << "Invaild external clock source.\n"
			<< ErrorWord(line, position_, length_) << "\n";
	} else if (status_ == 300) {
		ss << "Generate error.\n";
	} else {
		ss << "Undefined error: " << status_ << "\n";
	}
	return ss.str();
}



};