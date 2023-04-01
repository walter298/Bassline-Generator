#pragma once

#include <string>
#include <vector>
#include <array>
#include <map>
#include <algorithm>
#include <iostream>
#include <memory>

namespace msc {
	inline std::map<char, int> pitches = {
		{'C', 0},
		{'D', 2},
		{'E', 4},
		{'F', 5},
		{'G', 7},
		{'A', 9},
		{'B', 11}
	};

	inline constexpr int LOWEST_BASS_PITCH = 34; //lowest pitch the bass can go to
	inline constexpr int HIGHEST_BASS_PITCH = 48; //highest pitch the bass can go to
	inline constexpr int LARGEST_BASS_LEAP = 7; //bass can't leap more than a fifth

	struct Note {
		std::string name = "";
		int pitch = 0;
		int duration = 0; //whole = 16, half = 8, etc
	};

	inline constexpr int ROOT = 0;
	inline constexpr int FIRST = 1;
	inline constexpr int SECOND = 2;
	inline constexpr int THIRD = 3;

	struct Chord {
		int degree = 0;
		std::vector<Note> notes;
		std::vector<Chord*> destinations;
		int inversion = ROOT;
	};

	int getInversion(const std::vector<Note>& chord, const Note& bass);

	inline constexpr int SECONDARY_DOM_DEGREE = -1;

	class Key {
	private:
		//distance in halfsteps of scale degrees from to tonic depending on key quality
		static constexpr std::array<int, 7> majorPitches{
			2, 4, 5, 7, 9, 11, 12
		};
		static constexpr std::array<int, 7> harmonicMinorPitches{
			2, 3, 5, 7, 8, 11, 12
		};

		//chords corresponding to scale degrees
		std::array<std::shared_ptr<Chord>, 7> m_chords;

		//special chords
		std::shared_ptr<Chord> m_secondaryDominant;
	public:
		enum class KeyQuality {
			MAJOR,
			HARMONIC_MINOR
		};
		bool major = false;

		/*Quality indicates whether key is major or minor. Tonic represents the starting note 
		of the scale.*/
		Key(KeyQuality quality, Note tonic);

		std::weak_ptr<Chord> operator[](int idx) const;

		std::vector<std::weak_ptr<Chord>> possibleChords(std::vector<Note> notes) const;
	};

	//inline Key BFlatMajor{ Key::KeyQuality::MAJOR, { "D", 2} };
}

