#include "output_writer.h"

void msc::writeToOutputFile(std::string filePath, const std::vector<Note>& bassLine, const std::vector<Chord>& chords, bool major) {
	std::fstream file;
	file.open(filePath);

	std::vector<std::string> parsedLines, originalLines;

	//put all the lines in the file into lines
	std::string line;
	while (std::getline(file, line)) {
		parsedLines.push_back(line);
		originalLines.push_back(line);
	}

	file.close();

	//erase all whitespace from the beginning of each line. 
	for (auto& line : parsedLines) {
		line.erase(std::remove_if(line.begin(), line.end(), std::isspace), line.end());
	}

	//get line where the # of beats is specified
	StringVecIt it = std::find_if(parsedLines.begin(), parsedLines.end(), [](const std::string& string) { return string.contains("<beats>"); });
	int beatCount = std::stoi(enclosedString(*it, '>', '<')) * 4;

	auto writeNoteData = [](const Note& note) -> std::vector<std::string> { 
		std::vector<std::string> ret;

		ret.push_back("<note>"); //add opening note attribute
		ret.push_back("<pitch>"); //add opening pitch attribute
		std::string stepAttribute = makeAttribute("step", std::string{ note.name[0] });
		ret.push_back(stepAttribute); //add step attribute

		/*if there are accidentals in the note, add alterations attribute and 
		calculate octave based on accidental*/
		if (note.name.size() > 1) { 
			int direction;
			if (note.name[1] == 'b') {
				direction = -1;
			} else {
				direction = 1;
			}
			int pitchAlteration = direction * (note.name.size() - 1);
			std::string alterAttribute = makeAttribute("alter", std::to_string(pitchAlteration));
			ret.push_back(alterAttribute); //add alter attribute

			int octave = (note.pitch + ((pitchAlteration) - pitches[note.name[0]])) / 12;
			std::string octaveAttribute = makeAttribute("octave", std::to_string(octave));
			ret.push_back(octaveAttribute); //add specially calculated octave attribute
		} else {
			int octave = note.pitch / 12;
			std::string octaveAttribute = makeAttribute("octave", std::to_string(octave));
			ret.push_back(octaveAttribute); //add normally calculated octave attribute
		}

		ret.push_back("</pitch>"); //add closing pitch attributes

		std::string durationAttribute = makeAttribute("duration", "1"); //duration will always equal 1
		ret.push_back(durationAttribute); //add duration attribute
		
		std::string voiceAttribute = makeAttribute("voice", "1");
		ret.push_back(voiceAttribute); //add voice attribute

		std::string noteType;
		switch (note.duration) {
		case 2:
			noteType = "eighth";
			break;
		case 4:
			noteType = "quarter";
			break;
		case 8:
			noteType = "half";
			break;
		case 16:
			noteType = "whole";
			break;
		}

		std::string noteTypeAttribute = makeAttribute("type", noteType);
		ret.push_back(noteTypeAttribute);
		ret.push_back("<staff>1</staff>");
		ret.push_back("</note>"); //add closing note attribute

		return ret;
	};

	auto writeChordData = [major](const Chord& chord) {
		StringVec ret;

		//write chord data
		ret.push_back("<harmony placement=\"below\">");
		ret.push_back(makeAttribute("function", chordNames[{chord.degree, major}]));
		if (chord.degree == SECONDARY_DOM_DEGREE) { //put second function if V/V
			ret.push_back(makeAttribute("function", chordNames[{chord.degree, major}]));
		}
		std::string name;
		if (chord.inversion == 3 && chord.degree == 5) {
			name = "dominant";
		} else if (islower(chordNames[{chord.degree, major}][0])) {
			name = "minor";
		} else {
			name = "major";
		}
		if (chord.degree != 5 && (chord.inversion == 3 || (chord.inversion == 2 && (chord.degree == 2 && chord.degree == 5)))) {
			name.append("-seventh");
		}
	
		ret.push_back(makeAttribute("kind", name));
		ret.push_back(makeAttribute("inversion", std::to_string(chord.inversion)));
		ret.push_back(makeAttribute("staff", "1"));
		ret.push_back("</harmony>");

		return ret;
	};

	//find the measure where the rests start
	it = std::find(parsedLines.begin(), parsedLines.end(), "<rest/>");
	skipToLine(it, parsedLines.begin(), "measurenumber", - 1);
	
	int measureCount = std::stoi(enclosedString(*it, '"', '"'));

	int beatsPassed = 0; //# of beats not taken up by rests in the current measure

	//write measure attributes
	std::vector<std::string> measureAttributes; 
	for (size_t i = 0; i < bassLine.size(); i++) {
		if (beatsPassed == beatCount) {
			beatsPassed = 0;
			measureAttributes.push_back("</measure>");
			measureCount++;
		}
		if (beatsPassed == 0) {
			std::string measureNameAttribute = std::string("<measure number=\"") + std::to_string(measureCount) + "\">";
			measureAttributes.push_back(measureNameAttribute);
		}

		auto chordAttribute = writeChordData(chords[i]);
		measureAttributes.insert(measureAttributes.end(), chordAttribute.begin(), chordAttribute.end());

		auto noteAttribute = writeNoteData(bassLine[i]);
		measureAttributes.insert(measureAttributes.end(), noteAttribute.begin(), noteAttribute.end());

		beatsPassed += bassLine[i].duration;
	}

	for (const Note& note : bassLine) {
		
	}
	measureAttributes.push_back("</measure>");

	auto newIt = std::find_if(originalLines.begin(), originalLines.end(), 
		[](const std::string& str) { return str.contains("<rest/>"); }
	);
	skipToLine(newIt, originalLines.begin(), "number", -1);
	auto partIt = newIt; //INTENTIONAL COPY
	skipToLine(partIt, originalLines.end(), "part", 1);

	originalLines.erase(newIt, partIt);
	auto l = originalLines.back();
	auto pl = originalLines[originalLines.size() - 2];

	originalLines.pop_back();
	originalLines.pop_back();

	originalLines.insert(originalLines.end(), measureAttributes.begin(), measureAttributes.end());

	originalLines.push_back(pl);
	originalLines.push_back(l);

	std::remove("output.musicxml");
	std::ofstream output("output.musicxml");

	for (auto& line : originalLines) {
		output << line << std::endl;;
	}
	
	output.close();
}