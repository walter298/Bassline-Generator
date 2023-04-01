#include "bassline_maker.h"

std::optional<int> msc::ChordTree::ChordNode::legalBassPitch(Key& key, const Chord& destination) {
	Note bass = destination.notes[static_cast<size_t>(destination.inversion)]; //widening conversion

	//the soprano and bass should never have the same note
	if (bass.name == sopranoLine[noteIdx + 1].name && 
		writtenBaseNotes.back().name == sopranoLine[noteIdx].name)
	{
		return {};
	}

	//resolve the leading tone
	Note leadingTone = key[6].lock()->notes[0];
	Note tonic = key[0].lock()->notes[0];

	bool leadingToneResolutionNeed = false;

	if (writtenBaseNotes.back().name == leadingTone.name) {
		std::cout << writtenBaseNotes.back().name << " needs to resolve to " << tonic.name << std::endl;
		leadingToneResolutionNeed = true;
	}
	if (leadingToneResolutionNeed && bass.name != tonic.name) {
		return {};
	}

	/*Now check to see if we can make a pitch that is in the range of the bass, makes a legal bass leap, 
	and does not create a parallel 5th with the soprano voice*/
	int sopranoInterval = sopranoLine[noteIdx + 1].pitch - sopranoLine[noteIdx].pitch;

	while (true) {
		bass.pitch += 12; //add octave to note
		if (bass.pitch > HIGHEST_BASS_PITCH) { //when we have tried every single legal pitch
			break;
		}
		if (bass.pitch < LOWEST_BASS_PITCH) { //keep moving note up an octave if it is too low
			continue;
		}
	
		if (std::abs(bass.pitch - writtenBaseNotes.back().pitch) > LARGEST_BASS_LEAP) {
			continue;
		}
		int bassInterval = bass.pitch - writtenBaseNotes.back().pitch;
		if (std::abs(bassInterval) == 6) { //forbid tritone
			continue;
		}
		if (leadingToneResolutionNeed) {
			if (bassInterval != 1) {
				continue;
			}
		}
	
		//chordal seventh resolution test
		if (m_chord->inversion == 3) {
			if (bassInterval != -1) {
				continue;
			}
		}

		if (bassInterval == sopranoInterval && sopranoInterval == 7) { //if we have parallel fifths
			continue;
		} 

		return bass.pitch;
	}

	return {};
}

bool msc::ChordTree::ChordNode::validInversion() {
	if (noteIdx == sopranoLine.size() - 1 && m_chord->inversion != 0) { //always make the last chord in root position
		return false;
	}

	//notes preceeded by a 6 chord must be in root position
	if (previous->m_chord->degree == 6 && m_chord->inversion != 0) {
		return false;
	}

	if (previous->m_chord->degree == SECONDARY_DOM_DEGREE && m_chord->inversion == 3) {
		return false;
	}

	switch (m_chord->degree) {
	case -1: 
		return m_chord->inversion != THIRD; //no V of V chords with a 7th allowed
		break;
	case 1:
		//6/4  and 3rd inversion 1 chords are banned!
		return m_chord->inversion != SECOND && m_chord->inversion != THIRD;
		break;
	case 4:
		return m_chord->inversion != THIRD; //no 4 chords with a 7th
		break;
	case 6:
		//six chord in first inversion must be preceeded by a V/V
		if (m_chord->inversion == FIRST) {
			return previous->m_chord->degree == SECONDARY_DOM_DEGREE;
		} else { //otherwise, a 6 chord must be in root position
			return m_chord->inversion == ROOT && previous->m_chord->degree != SECONDARY_DOM_DEGREE;
		}
		break;
	case 7:
		return m_chord->degree == FIRST; //seventh chords must be in 1st inversion
		break;
	}

	return true;
}

std::vector<msc::ChordTree::ChordNode*> msc::ChordTree::ChordNode::generateDestinations(Key* key) {
	std::vector<ChordNode*> ret;

	std::vector<Chord*> chordDestinations;
	
	//Chord factory 
	auto makeChordWithInversion = [](Chord* chord, int inversion) {
		Chord* inverted = new Chord(*chord);
		inverted->inversion = inversion;
		return inverted;
	};

	auto destinations = key->possibleChords(std::vector{ sopranoLine[noteIdx + 1] });

	const auto& legalChordMoves = m_chord->destinations;

	//for each chord, push back a chord with all possible inversions
	for (std::weak_ptr<Chord> dest : destinations) {
		if (std::find_if(legalChordMoves.begin(), legalChordMoves.end(),
			[dest](Chord* chord) { return chord->degree == dest.lock().get()->degree; }) != legalChordMoves.end())
		{
			for (int i = 0; i < 4; i++) {
				chordDestinations.push_back(makeChordWithInversion(dest.lock().get(), i));
			}
		}
	}

	//make new nodes from chords
	for (Chord* chord : chordDestinations) {
		ret.push_back(new ChordNode{ chord, true, noteIdx + 1 });
	}

	return ret;
}

void msc::ChordTree::explore() {
	//ChordNode* current = m_cursor;
	if (m_cursor == nullptr) {
		std::cout << "I couldn't solve this one.\n";
		exit(0);
	}

	//generate destinations if we haven't already
	if (!m_cursor->generatedDestinations) {
		m_cursor->destinations = m_cursor->generateDestinations(m_key);
		m_cursor->generatedDestinations = true;
	}

	//put unexplored destinations in a vector
	std::vector<ChordNode*> unexploredDestinations;

	for (ChordNode* node : m_cursor->destinations) {
		if (!node->explored) {
			unexploredDestinations.push_back(node);
		}
	}

	//if there are no legal chord moves, backtrack
	if (unexploredDestinations.size() == 0) { 
		std::cout << "backtracking\n";
		m_cursor->explored = true;
		m_cursor = m_cursor->previous;
		ChordNode::writtenBaseNotes.pop_back();
		ChordNode::chords.pop_back();
		explore();
		return;
	}

	//random number device, generator, and distibution to pick a random element from unexploredDestinations
	static std::random_device dev;
	static std::mt19937 rng(dev());
	std::uniform_int_distribution<size_t> dist(0, unexploredDestinations.size() - 1);

	size_t randomDestIdx = dist(rng);
	
	//pick a random destination
	ChordNode* randomDest = unexploredDestinations[randomDestIdx];
	randomDest->previous = m_cursor; 

	if (!randomDest->validInversion()) {
		randomDest->explored = true;
		explore();
		return;
	} 

	auto pitch = m_cursor->legalBassPitch(*m_key, *randomDest->m_chord);

	if (!pitch.has_value()) {
		randomDest->explored = true;
		explore();
		return;
	}

	m_cursor = randomDest;
	
	//add note to bassline
	ChordNode::writtenBaseNotes.push_back(
		{ randomDest->m_chord->notes[randomDest->m_chord->inversion].name, pitch.value(), 
		  ChordNode::sopranoLine[randomDest->noteIdx].duration }
	);
	ChordNode::chords.push_back(*randomDest->m_chord);

	if (ChordNode::writtenBaseNotes.size() == ChordNode::chordCountGoal) {
		return;
	} else {
		explore();
		return;
	}
}

msc::OutputData msc::ChordTree::getPath() 
{
	explore();

	//erase the data that was not written
	auto& bassLine = ChordTree::ChordNode::writtenBaseNotes;
	bassLine.erase(bassLine.begin()); 
	auto& chords = ChordTree::ChordNode::chords;
	chords.erase(chords.begin());

	return std::make_pair(bassLine, chords);
}

msc::ChordTree::ChordTree(Key* key, std::vector<Note>& sopranoLine, Note firstBassNote, Chord* chord, 
						  size_t startSopranoNoteIdx, size_t chordCountGoal) 
{
	m_key = key;

	m_sentinel = new ChordNode{ chord, true, startSopranoNoteIdx };
	m_cursor = m_sentinel;
	
	ChordNode::writtenBaseNotes.push_back(firstBassNote); //set first bass note
	ChordNode::chords.push_back(*chord); //set first chord
	ChordNode::chordCountGoal = chordCountGoal + 1; //we add one because the starting chord does not count
	ChordNode::sopranoLine = sopranoLine;
}

msc::OutputData msc::writeBassLine(Key& key, std::vector<Note>& sopranoLine, std::vector<Note>& bassLine, int finalDegree)
{
	int preBassLineLength = 0; //# of beats the pre-given bassline goes for
	for (int i = 0; i < bassLine.size(); i++) {
		preBassLineLength += bassLine[i].duration;
	}

	size_t lastBassNoteIdx = bassLine.size() - 1; //index of the final bass note

	//start writing bassline where the given bassline ends
	size_t startSopranoNoteIdx = 0;
	int beatCount = 0;
	for (startSopranoNoteIdx; startSopranoNoteIdx < sopranoLine.size(); startSopranoNoteIdx++) {
		beatCount += sopranoLine[startSopranoNoteIdx].duration;
		if (beatCount == preBassLineLength) {
			break;
		} else if (beatCount > preBassLineLength) {
			std::cout << "Error: pre-given bassline must end in alignment with soprano voice\n";
			exit(0);
		}
	}

	/*# of notes we hope to have in our path. This is equal to the # of notes
	between the first unacompannied soprano note to the last soprano note (inclusive)*/
	size_t nodeTraversalGoal = (sopranoLine.size() - 1) - startSopranoNoteIdx;
	//std::cout << "soprano notes: " << sopranoLine.size() << std::endl;

	std::vector<Note> lastChordNotes; //final notes written before blank bassline
	if (preBassLineLength > 0) {
		std::cout << "last bass note index: " << lastBassNoteIdx << std::endl;
		//std::cout << "Vector size: " << bassLine.size() << std::endl;
		lastChordNotes = { bassLine[lastBassNoteIdx], sopranoLine[startSopranoNoteIdx] };
	} else {
		lastChordNotes = { sopranoLine[startSopranoNoteIdx] };
	}

	auto startChord = key[finalDegree - 1];

	ChordTree chordTree{ &key, sopranoLine, bassLine.back(), startChord.lock().get(), startSopranoNoteIdx, nodeTraversalGoal };

	auto data = chordTree.getPath();
	
	return data;
}