#include "parser.h"
#include "bassline_maker.h"
#include "output_writer.h"

int main() {
	msc::ResultData info;
	std::string fileName;
	std::cout << "Enter the name of your musicxml score file: ";
	std::cin >> fileName;
	std::replace(fileName.begin(), fileName.end(), '\\', '/');
	info = msc::parseMeasures(fileName);
	auto& [key, soprano, bass, degree] = info.value();

	auto [newBassLine, chords] = msc::writeBassLine(key, soprano, bass, degree);

	msc::writeToOutputFile(fileName, newBassLine, chords, key.major);
	/*try {
		info = msc::parseMeasures("input_7.musicxml");
		auto& [key, soprano, bass, degree] = info.value();
		std::cout << "Degree: " << degree << std::endl;
		msc::writeBassLine(key, soprano, bass, degree);
	} catch (std::exception e) {
		std::cout << "Error: Bad input. Check your score for errors\n";
		std::cout << e.what() << std::endl;
	}*/
}