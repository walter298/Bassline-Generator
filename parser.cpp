#include "parser.h"

msc::ResultData msc::parseMeasures(std::string path) 
{
	std::ifstream file;
	file.open(path);

	if (!file.is_open()) {
		std::cout << "Error: file is not open\n";
		return {};
	}

	std::vector<std::string> lines;
	std::string cLine;

	//put all the lines of the file input into a list
	while (std::getline(file, cLine)) {
		lines.push_back(cLine);
	}
	file.close(); //we are done using the input file

	//erase all whitespace from the beginning of each line. 
	for (auto& line : lines) {
		line.erase(std::remove_if(line.begin(), line.end(), std::isspace), line.end());
	}

	StringVecIt linesIt = lines.begin(); 

	auto loadPartData = [linesIt](StringVec& data) mutable {
		while (true) {
			data.push_back(*linesIt);
			if (linesIt->contains("</part>")) {
				linesIt ++;
				break;
			}
			linesIt++;
		}
	};

	std::string keyName; //name of the key
	int pitchOfKey = 0;
	auto keyNameIt = std::find_if(lines.begin(), lines.end(),
		[](const std::string& line) { return line.contains("<words>"); }
	);
	if (keyNameIt == lines.end()) {
		std::cout << "Error: no key was provided! Go back to your score in flat, hit the text tab, then hit annotation,\n";
		std::cout << "and then write the name of the key, uppercase for major and lowercase for harmonic minor.\n";
		std::cout << "Ex: C#  = C# major, d = d harmonic minor.\n";
		return {};
	}
	keyName = enclosedString(*keyNameIt, '>', '<');

	auto finalChordNameIt = std::find_if(lines.begin(), lines.end(),
		[](const std::string& line) { return line.contains("<function>"); }
	);
	if (finalChordNameIt == lines.end()) {
		std::cout << "Error: you need to write the final chord before the bassline ends\n";
		exit(0);
	}

	std::string finalStringName = enclosedString(*finalChordNameIt, '>', '<');
	for (const char& chr : *finalChordNameIt) {
		if (isalpha(chr)) {
			finalStringName.push_back(chr);
		} else {
			break;
		}
	}
	std::cout << "Final string name: " << finalStringName << std::endl;
	int finalDegree = numeralsToDegrees.at(finalStringName);
	std::cout << "Parsed this degree: " << finalDegree << std::endl;

	Key::KeyQuality quality = Key::KeyQuality::MAJOR;

	pitchOfKey = pitches.at(std::toupper(keyName[0]));

	if (keyName.size() > 1) { //if we have accidentals in key name, modify key pitch accordingly
		int pitchMod = 0; //1 for sharp, -1 for flats
		if (keyName[1] == 'b') {
			pitchMod = -1;
		} else {
			pitchMod = 1;
		}
		pitchOfKey += (pitchMod * static_cast<int>(keyName.size() - 1));
	}
	if (std::islower(keyName[0])) { //make key harmonic minor if the first character is lowercase
		quality = Key::KeyQuality::HARMONIC_MINOR;
	}

	Key key{ quality, { std::string { static_cast<char>(std::toupper(keyName[0])) }, pitchOfKey } };

	std::vector<Note> bass, tenor, alto, soprano;

	skipToLine(linesIt, lines.end(), "<partid", 1);

	/*
	* For each note attribute section, parse the following lines in order: 
	* 1. The name (A, B, C, etc) from the step attribute. 
	* 2. The octave from the octave attribute. Add the pitch of current note by 8 * octave
	* 3. The type of note (quarter, eighth, etc) which will affect its duration
	*/
	auto parsePartData = [&](std::vector<Note>& notes) {
		StringVec partData;
		loadPartData(partData);

		auto partIt = partData.begin();

		while (true) {
			skipToLine(partIt, partData.end(), "<step>", 1);
			if (partIt == partData.end()) {
				break;
			}

			std::string name;
			name.push_back(enclosedString(*partIt, '>', '<').at(0));
			int pitch = pitches.at(name[0]);

			skipToEitherLine(partIt, partData.end(), "alter", "octave");
			if (partIt->contains("alter")) {
				int pitchAlteration = std::stoi(enclosedString(*partIt, '>', '<')); //1 for sharp and -1 for flat
				pitch += pitchAlteration;

				char accidental = 'b';
				if (pitchAlteration >= 1) {
					accidental = '#';
				}
				for (int i = 0; i < std::abs(pitchAlteration); i++) {
					name.push_back(accidental);
				}
				skipToLine(partIt, partData.end(), "octave", 1);
				auto enString = enclosedString(*partIt, '>', '<');
				int octave = std::stoi(enclosedString(*partIt, '>', '<'));
				if (pitch + (12 * octave) < 0) {
					std::cout << "Iterator: " << *partIt << std::endl;
					std::cout << "Enclosed String: " << enString << std::endl;
					std::cout << "Pitch: " << pitch << std::endl;
					std::cout << "Octave: " << octave << std::endl;
				}
				pitch += (12 * octave);
				//std::cout << "Altered pitch: " << pitch << std::endl;
			} else { //if *partIt->contains("octave")
				int octave = std::stoi(enclosedString(*partIt, '>', '<'));
				if (pitch + (12 * octave) < 0) {
					std::cout << "Pitch: " << pitch << std::endl;
					std::cout << "Octave: " << octave << std::endl;
				}
				pitch += (12 * octave);
				//std::cout << "Altered pitch: " << pitch << std::endl;
			}
			skipToLine(partIt, partData.end(), "<type>", 1);
			std::string type = enclosedString(*partIt, '>', '<');

			int duration = 0;

			if (type == "eighth") {
				duration = 2;
			} else if (type == "quarter") {
				duration = 4;
			} else if (type == "half") {
				duration = 8;
			} else if (type == "whole") {
				duration = 16;
			}
			notes.emplace_back(name, pitch, duration);

			//std::cout << "Parsed: " << duration << std::endl;
		} 
	}; 

	parsePartData(soprano);
	parsePartData(bass);

	/*std::cout << "Soprano Line: \n";
	for (Note& note : soprano) {
		std::cout << "Name: " << note.name << std::endl;
		std::cout << "Duration: " << note.duration << std::endl;
		std::cout << "Pitch: " << note.pitch << std::endl;
		std::cout << std::endl;
	}

	std::cout << "Bass line: \n";
	for (Note& note : bass) {
		std::cout << "Name: " << note.name << std::endl;
		std::cout << "Duration: " << note.duration << std::endl;
		std::cout << "Pitch: " << note.pitch << std::endl;
		std::cout << std::endl;
	}*/

	return make_tuple(key, soprano, bass, finalDegree);
}