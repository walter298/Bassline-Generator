#pragma once

#include <vector>
#include <string>
#include <algorithm>

namespace msc {
	using StringVecIt = std::vector<std::string>::iterator;
	using StringVec = std::vector<std::string>;

	//move iterator until its value contains the substring A or B, or reaches the end
	void skipToEitherLine(StringVecIt& it, StringVecIt endIt, std::string substrA, std::string substrB);

	//move iterator until its value contains the substring substr without bounds checking
	void skipToLine(StringVecIt& it, StringVecIt endIt, std::string substr, int decerement);

	//returns the enclosed substring sandwiched between two of the given characters
	std::string enclosedString(std::string str, char chrLeft, char chrRight);

	std::string makeAttribute(std::string attributeName, std::string bracketedString);
}