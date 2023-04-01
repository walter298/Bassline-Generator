#pragma once

#include <fstream>
#include <string>

#include "types.h"
#include "file_util.h"

namespace msc {
	inline std::map<std::pair<int, bool>, std::string> chordNames{
		//V/V is always V/V regardless of chord quality
		{ {-1, true }, "V"},
		{ {-1, false }, "V"},

		//major chords
		{ { 1, true }, "I" },
		{ { 2, true }, "ii" },
		{ { 3, true }, "iii" },
		{ { 4, true }, "IV" },
		{ { 5, true }, "V" },
		{ { 6, true }, "vi" },
		{ { 7, true }, "vii" },

		//minor chords
		{ { 1, false }, "i" },
		{ { 2, false }, "ii" },
		{ { 3, false }, "III" },
		{ { 4, false }, "iv" },
		{ { 5, false }, "V" },
		{ { 6, false }, "VI" },
		{ { 7, false }, "vii" }
	};

	void writeToOutputFile(std::string filePath, const std::vector<Note>& bassLine, const std::vector<Chord>& chords, bool major);
}