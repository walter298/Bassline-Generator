#include "file_util.h"

//move iterator until its value contains the substring A or B, or reaches the end
void msc::skipToEitherLine(StringVecIt& it, StringVecIt endIt, std::string substrA, std::string substrB) {
	while (it != endIt && !it->contains(substrA) && !it->contains(substrB)) {
		it++;
	}
}

//move iterator until its value contains the substring substr without bounds checking
void msc::skipToLine(StringVecIt& it, StringVecIt endIt, std::string substr, int decrement) {
	while (it != endIt && !it->contains(substr)) {
		it += decrement;
	}
};

//returns the enclosed substring sandwiched between two of the given characters
std::string msc::enclosedString(std::string str, char chrLeft, char chrRight) {
	std::string ret;
	auto start = std::find(str.begin(), str.end(), chrLeft) + 1;
	for (; *start != chrRight; start++) {
		ret.push_back(*start);
	}
	return ret;
};

std::string msc::makeAttribute(std::string attributeName, std::string bracketedString) {
	return std::string("<") + attributeName + ">" + bracketedString + "</" + attributeName + ">";
}