#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <optional>
#include <tuple>

#include "types.h"
#include "file_util.h"

namespace msc {
	inline std::map<std::string, int> numeralsToDegrees{
		{ "I", 1 },
		{ "ii", 2 },
		{ "IV", 4 },
		{ "iv", 4 },
		{ "V", 5 },
		{ "v", 5 },
		{ "VI", 6 },
		{ "vi", 6 },
	};

	//Data contains a key (e.g. D major), the soprano line, the bassline, the degree of the last written chord
	using ResultData = std::optional<std::tuple<Key, std::vector<Note>, std::vector<Note>, int>>;
	ResultData parseMeasures(std::string path);
}
