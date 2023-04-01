#pragma once

#include <random>
#include <optional>

#include "types.h"

namespace msc {
	using OutputData = std::pair<std::vector<Note>, std::vector<Chord>>;

	class ChordTree {
	private:
		//size_t m_endSopranoNoteIdx = 0;

		struct ChordNode {
			Chord* m_chord;

			bool ownedByTree = false; //flags whether the node should be deleted when the chord tree is destroyed

			size_t noteIdx = 0;//current index of the soprano line

			static inline size_t chordCountGoal = 0; //the # of chords we need to have in the bass line
			static inline std::vector<Note> sopranoLine; //non-owning
			static inline std::vector<Note> writtenBaseNotes;
			static inline std::vector<Chord> chords;

			bool explored = false;
			bool generatedDestinations = false;

			std::vector<ChordNode*> destinations;

			ChordNode* previous = nullptr; //node that was visited before this node
			ChordNode* next     = nullptr;

			std::optional<int> legalBassPitch(Key& key, const Chord& destination);
			bool validInversion();
			std::vector<ChordNode*> generateDestinations(Key* key);

			inline void printData() {
				for (Note& note : m_chord->notes) {
					std::cout << note.name << " ";
				}
				std::cout << std::endl;
				std::cout << "Explored Flag: " << explored << std::endl;
				std::cout << "Previous node is nullptr flag: " << (previous == nullptr) << std::endl;
			}
		};

		ChordNode* m_sentinel = nullptr;
		ChordNode* m_cursor   = nullptr;
		ChordNode* m_current  = nullptr;
		
		Key* m_key = nullptr;

		void explore();
	public:
		OutputData getPath();

		ChordTree(Key* key, std::vector<Note>& sopranoLine, Note firstBassNote, Chord* chord, 
			      size_t startSopranoNoteIdx, size_t chordCountGoal);
	};

	OutputData writeBassLine(Key& key, std::vector<Note>& sopranoLine, std::vector<Note>& bassLine, int finalDegree);
}