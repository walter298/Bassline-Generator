#include "types.h"

//returns what inversion a chord is in from a given bass note
int msc::getInversion(const std::vector<Note>& notes, const Note& bass) {
	for (size_t i = 0; i < notes.size(); i++) {
		if (bass.name == notes[i].name) {
			return static_cast<int>(i);
		}
	}

	return ROOT;
}

msc::Key::Key(KeyQuality quality, Note tonic)
{
	if (quality == KeyQuality::MAJOR) {
		major = true;
	} else {
		major = false;
	}

	std::array<Note, 7> notes = { tonic }; //notes in the scale
	std::ranges::fill(notes.begin() + 1, notes.end(), Note()); //fill all non-tonic notes with default values

	auto setPitches = [&notes](const std::array<int, 7>& pitches) {
		size_t intervalIdx = 0;
		for (size_t i = 1; i < pitches.size(); i++, intervalIdx++) {
			notes[i].pitch = notes[0].pitch + pitches[intervalIdx];
		}
	};

	//Set pitches of each note depending on quality of key
	if (quality == KeyQuality::MAJOR) {
		setPitches(majorPitches);
	} else {
		setPitches(harmonicMinorPitches);
	}
	
	/*find what letter the supertonic's name by finding position of
	tonic's name in the pitch map and adding 1*/
	auto noteNameIndex = pitches.begin();

	for (auto it = pitches.begin(); it != pitches.end(); it++) {
		if (tonic.name[0] == it->first) {
			//make the next iterator the first element if we have reached the end of pitches
			//otherwise just make the next iterator be the next element
			if (std::next(it) == pitches.end()) { 
				noteNameIndex = pitches.begin();
			} else {
				noteNameIndex = std::next(it);
			}
		}
	}

	//pitches of the scale in sorted order, from do-ti
	std::map<char, int> sortedPitches = { { tonic.name[0], pitches[tonic.name[0]] } };

	//give each note a name, ignoring accidentals. Add ordered scale degrees to sortedPitches
	for (size_t i = 1; i < notes.size(); i++) {
		if (noteNameIndex == pitches.end()) { //wrap back around to C 
			noteNameIndex = pitches.begin();
		}
		char name = noteNameIndex->first;
		notes[i].name = std::string{ name };

		//add sorted name and pitch element
		int pitch = pitches[name];
		if (pitch < sortedPitches.rbegin()->second) { //add octave to make pitches sorted 
			pitch += 12;
		}
		sortedPitches.emplace(name, pitch);
		noteNameIndex++;
	}

	/*Add accidentals to each note name if the pitch of the given note
	doesn't match the pitch in sortedPitches*/
	for (Note& note : notes) {
		int mapPitchDiff = sortedPitches[note.name[0]] - sortedPitches[notes[0].name[0]];
		int scalePitchDiff = note.pitch - sortedPitches[notes[0].name[0]];
		size_t accCount = std::abs(scalePitchDiff - mapPitchDiff); //# of accidentals to add

		if (scalePitchDiff > mapPitchDiff) {
			for (size_t i = 0; i < accCount; i++) {
				note.name.push_back('#');
			}
		} else if (scalePitchDiff < mapPitchDiff) {
			for (size_t i = 0; i < accCount; i++) {
				note.name.push_back('b');
			}
		}
	}

	//populate the chords
	for (size_t i = 0; i < notes.size(); i++) {
		size_t thirdIdx = (i + 2) % notes.size(); //idx of third of triad
		size_t fifthIdx = (i + 4) % notes.size(); //idx of fifth of triad

		if (i == 1 || i == 4) { //if we can write a seven chord with 2 or 5, add the seventh to the current chord
			size_t seventhIdx = (i + 6) % notes.size();
			m_chords[i] = std::make_shared<Chord>(
				Chord{ static_cast<int>(i + 1), { notes[i], notes[thirdIdx], notes[fifthIdx], notes[seventhIdx] } }
			);
		} else { //otherwise, just make a plain chord with a root, third, and fifth
			m_chords[i] = std::make_shared<Chord>(
				Chord{ static_cast<int>(i + 1), { notes[i], notes[thirdIdx], notes[fifthIdx] } }
			);
		}
	}

	//generate special chords (just secondary dominant for now)
	std::string secondDomName = notes[3].name;
	if (notes[3].name.back() == 'b') {
		secondDomName.pop_back();
	} else {
		secondDomName.push_back('#');
	}
	m_secondaryDominant = std::make_shared<Chord>(
		Chord { SECONDARY_DOM_DEGREE, { notes[1], { secondDomName, notes[3].pitch + 1 }, notes[5] } }
	);

	m_chords[0]->destinations = { m_chords[3].get(), m_chords[4].get(), m_chords[5].get(), m_chords[6].get(), m_secondaryDominant.get() };
	m_chords[1]->destinations = { m_chords[4].get(), m_chords[6].get() };
	m_chords[3]->destinations = { m_chords[0].get(), m_chords[1].get(), m_chords[4].get() };
	m_chords[4]->destinations = { m_chords[0].get(), m_chords[5].get(), m_secondaryDominant.get() };
	m_chords[5]->destinations = { m_chords[1].get(), m_chords[3].get(), m_chords[4].get() };
	m_chords[6]->destinations = { m_chords[0].get() };
	m_secondaryDominant->destinations = { m_chords[4].get(), m_chords[5].get() };
}

std::weak_ptr<msc::Chord> msc::Key::operator[](int idx) const {
	return m_chords[static_cast<size_t>(idx)];
}

std::vector<std::weak_ptr<msc::Chord>> msc::Key::possibleChords(std::vector<Note> notes) const {
	std::vector<std::weak_ptr<Chord>> candidates;

	//checks to see whether the notes are part of a given chord
	auto isContained = [notes](const Chord* chord) {
		for (const Note& note : notes) { 
			bool sameName = false;
			for (const Note& chordNote : chord->notes) {
				if (chordNote.name == note.name) {
					sameName = true;
					break;
				}
			}
			if (!sameName) { //if note does not share a single name with the chord notes, return false
				return false;
			}
		}
		return true;
	};

	//skip 3 chords
	for (size_t i = 0; i < 2; i++) {
		if (isContained(m_chords[i].get())) {
			candidates.push_back(m_chords[i]);
		}
	}
	for (size_t i = 3; i < 7; i++) {
		if (isContained(m_chords[i].get())) {
			candidates.push_back(m_chords[i]);
		}
	}
	if (isContained(m_secondaryDominant.get())) {
		candidates.push_back(m_secondaryDominant);
	}

	return candidates;
}