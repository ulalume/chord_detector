#pragma once

#include <array>
#include <string>
#include <vector>
#include <cstring>

/**
 * Unified high-performance chord detector - optimized for real-time usage
 * Detects chord names from MIDI note numbers with optional slash chord support
 * Uses fixed-size arrays and bitmask pattern matching for O(1) lookups
 * No external dependencies, single header file
 *
 * Usage:
 *   // Basic chord detection
 *   std::string chord = get_chord_name({60, 64, 67}); // Returns "C"
 *
 *   // Slash chord detection
 *   std::string slash = get_chord_name({64, 67, 72}, false, true); // Returns "C/E"
 */

// Chord analysis result structure
struct ChordResult {
    std::string full_name;      // Complete chord name (e.g., "C/E", "Am7", "G")
    std::string chord_name;     // Main chord part (e.g., "C", "Am7", "G")
    std::string bass_note;      // Bass note (e.g., "E", "C", "G")
    bool is_slash_chord;        // True if bass != root
    int root_pitch_class;       // Root note (0-11)
    int bass_pitch_class;       // Bass note (0-11)
};

namespace ChordDetector {
    // Maximum number of notes we can handle
    constexpr int MAX_NOTES = 12;

    // Chord pattern definition using bitmask for O(1) lookup
    struct ChordPattern {
        uint16_t mask;          // Bitmask of intervals (bit N = interval N semitones)
        const char* name;       // Chord suffix
        int priority;           // Higher priority wins in case of ties
    };

    // Enhanced chord patterns with omit5 and add11 support
    constexpr ChordPattern CHORD_PATTERNS[] = {
        // Extended chords (highest priority due to specificity)
        {(1<<0)|(1<<2)|(1<<4)|(1<<5)|(1<<7)|(1<<10), "11",     100},  // Dom11: R,M2,M3,P4,P5,m7
        {(1<<0)|(1<<2)|(1<<4)|(1<<5)|(1<<7)|(1<<11), "maj11",  100},  // Maj11: R,M2,M3,P4,P5,M7
        {(1<<0)|(1<<2)|(1<<3)|(1<<5)|(1<<7)|(1<<10), "m11",    100},  // Min11: R,M2,m3,P4,P5,m7

        // 11th chords omit5
        {(1<<0)|(1<<2)|(1<<4)|(1<<5)|(1<<10), "11(omit5)",     95},   // Dom11 omit5: R,M2,M3,P4,m7
        {(1<<0)|(1<<2)|(1<<4)|(1<<5)|(1<<11), "maj11(omit5)",  95},   // Maj11 omit5: R,M2,M3,P4,M7
        {(1<<0)|(1<<2)|(1<<3)|(1<<5)|(1<<10), "m11(omit5)",    95},   // Min11 omit5: R,M2,m3,P4,m7

        // 9th chords
        {(1<<0)|(1<<2)|(1<<4)|(1<<7)|(1<<10), "9",      90},   // Dom9: R,M2,M3,P5,m7
        {(1<<0)|(1<<2)|(1<<4)|(1<<7)|(1<<11), "maj9",   90},   // Maj9: R,M2,M3,P5,M7
        {(1<<0)|(1<<2)|(1<<3)|(1<<7)|(1<<10), "m9",     90},   // Min9: R,M2,m3,P5,m7
        {(1<<0)|(1<<2)|(1<<3)|(1<<7)|(1<<11), "mM9",    90},   // MinMaj9: R,M2,m3,P5,M7

        // 9th chords omit5
        {(1<<0)|(1<<2)|(1<<4)|(1<<10), "9(omit5)",      85},   // Dom9 omit5: R,M2,M3,m7
        {(1<<0)|(1<<2)|(1<<4)|(1<<11), "maj9(omit5)",   85},   // Maj9 omit5: R,M2,M3,M7
        {(1<<0)|(1<<2)|(1<<3)|(1<<10), "m9(omit5)",     85},   // Min9 omit5: R,M2,m3,m7

        // 7th chords
        {(1<<0)|(1<<4)|(1<<7)|(1<<10), "7",      80},   // Dom7: R,M3,P5,m7
        {(1<<0)|(1<<4)|(1<<7)|(1<<11), "maj7",   80},   // Maj7: R,M3,P5,M7
        {(1<<0)|(1<<3)|(1<<7)|(1<<10), "m7",     80},   // Min7: R,m3,P5,m7
        {(1<<0)|(1<<3)|(1<<7)|(1<<11), "mM7",    80},   // MinMaj7: R,m3,P5,M7
        {(1<<0)|(1<<4)|(1<<6)|(1<<10), "7b5",    75},   // 7b5: R,M3,b5,m7
        {(1<<0)|(1<<3)|(1<<6)|(1<<10), "m7b5",   75},   // HalfDim: R,m3,b5,m7
        {(1<<0)|(1<<3)|(1<<6)|(1<<9), "o7",     75},   // Dim7: R,m3,b5,d7
        {(1<<0)|(1<<5)|(1<<7)|(1<<10), "7sus4",  70},   // 7sus4: R,P4,P5,m7
        {(1<<0)|(1<<2)|(1<<7)|(1<<10), "7sus2",  70},   // 7sus2: R,M2,P5,m7

        // 7th chords omit5
        {(1<<0)|(1<<4)|(1<<10), "7(omit5)",      72},   // Dom7 omit5: R,M3,m7
        {(1<<0)|(1<<4)|(1<<11), "maj7(omit5)",   72},   // Maj7 omit5: R,M3,M7
        {(1<<0)|(1<<3)|(1<<10), "m7(omit5)",     72},   // Min7 omit5: R,m3,m7
        {(1<<0)|(1<<3)|(1<<11), "mM7(omit5)",    72},   // MinMaj7 omit5: R,m3,M7

        // 6th chords
        {(1<<0)|(1<<4)|(1<<7)|(1<<9), "6",      78},   // Maj6: R,M3,P5,M6
        {(1<<0)|(1<<3)|(1<<7)|(1<<9), "m6",     78},   // Min6: R,m3,P5,M6

        // 6th chords omit5
        {(1<<0)|(1<<4)|(1<<9), "6(omit5)",      45},   // Maj6 omit5: R,M3,M6
        {(1<<0)|(1<<3)|(1<<9), "m6(omit5)",     45},   // Min6 omit5: R,m3,M6

        // Add11/Add4 chords (Enhanced patterns for C-E-F type chords)
        {(1<<0)|(1<<4)|(1<<5)|(1<<7), "add11",  65},   // Add11: R,M3,P4,P5 (same as add4)
        {(1<<0)|(1<<3)|(1<<5)|(1<<7), "madd11", 65},   // MinAdd11: R,m3,P4,P5
        {(1<<0)|(1<<4)|(1<<5), "add11(omit5)",  68},   // Add11 omit5: R,M3,P4 ← C-E-F case
        {(1<<0)|(1<<3)|(1<<5), "madd11(omit5)", 68},   // MinAdd11 omit5: R,m3,P4

        // Add9 chords
        {(1<<0)|(1<<2)|(1<<4)|(1<<7), "add9",   60},   // Add9: R,M2,M3,P5
        {(1<<0)|(1<<2)|(1<<3)|(1<<7), "madd9",  60},   // MinAdd9: R,M2,m3,P5

        // Add9 chords omit5
        {(1<<0)|(1<<2)|(1<<4), "add9(omit5)",   58},   // Add9 omit5: R,M2,M3
        {(1<<0)|(1<<2)|(1<<3), "madd9(omit5)",  58},   // MinAdd9 omit5: R,M2,m3

        // Sus chords with 7th
        {(1<<0)|(1<<5)|(1<<7)|(1<<10), "7sus4",  55},   // 7sus4: R,P4,P5,m7
        {(1<<0)|(1<<2)|(1<<7)|(1<<10), "7sus2",  55},   // 7sus2: R,M2,P5,m7

        // Basic triads
        {(1<<0)|(1<<4)|(1<<7), "",       60},   // Major: R,M3,P5
        {(1<<0)|(1<<3)|(1<<7), "m",      60},   // Minor: R,m3,P5
        {(1<<0)|(1<<4)|(1<<8), "+",      45},   // Aug: R,M3,m6
        {(1<<0)|(1<<3)|(1<<6), "o",      45},   // Dim: R,m3,b5

        // Sus chords
        {(1<<0)|(1<<2)|(1<<7), "sus2",   40},   // Sus2: R,M2,P5
        {(1<<0)|(1<<5)|(1<<7), "sus4",   40},   // Sus4: R,P4,P5
        {(1<<0)|(1<<2)|(1<<5), "sus2sus4", 30}, // Sus2sus4: R,M2,P4 (rare but possible)

        // Special case patterns for slash chord detection
        {(1<<0)|(1<<2)|(1<<5), "?",      35},   // R,M2,P4 - ambiguous, needs slash analysis ← C-D-F case

        // Power chords and incomplete
        {(1<<0)|(1<<7), "5",      30},   // Power: R,P5
        {(1<<0)|(1<<5), "sus4(omit5)", 25}, // Just R,P4 (sus4 without 5th)
        {(1<<0)|(1<<2), "sus2(omit5)", 25}, // Just R,M2 (sus2 without 5th)
        {(1<<0)|(1<<4), "",       20},   // Just R,M3
        {(1<<0)|(1<<3), "m",      20},   // Just R,m3
    };

    constexpr size_t NUM_PATTERNS = sizeof(CHORD_PATTERNS) / sizeof(ChordPattern);

    // Note name lookup tables
    constexpr const char* NOTE_NAMES_SHARP[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

    constexpr const char* NOTE_NAMES_FLAT[] = {
        "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
    };

    // Optimized interval calculation and sorting
    inline void insertion_sort(int* arr, int n) {
        for (int i = 1; i < n; ++i) {
            int key = arr[i];
            int j = i - 1;
            while (j >= 0 && arr[j] > key) {
                arr[j + 1] = arr[j];
                --j;
            }
            arr[j + 1] = key;
        }
    }

    // Create interval bitmask from sorted intervals
    inline uint16_t create_interval_mask(const int* intervals, int count) {
        uint16_t mask = 0;
        for (int i = 0; i < count; ++i) {
            if (intervals[i] >= 0 && intervals[i] < 12) {
                mask |= (1 << intervals[i]);
            }
        }
        return mask;
    }

    // Enhanced slash chord analysis
    inline ChordResult analyze_for_slash_chord(const int* midi_notes, int note_count,
                                             int bass_pitch_class, const char* const* note_names) {
        ChordResult best_result = {"", "", "", false, -1, -1};
        int best_priority = -1;

        // Try each note as potential root (excluding bass)
        for (int root_candidate = 0; root_candidate < 12; ++root_candidate) {
            if (root_candidate == bass_pitch_class) continue; // Skip bass note as root

            // Build intervals from this root candidate
            int intervals[MAX_NOTES];
            int interval_count = 0;
            bool has_root = false;

            for (int i = 0; i < note_count; ++i) {
                int midi = midi_notes[i];
                if (midi < 0 || midi > 127) continue;

                int pc = midi % 12;
                int interval = (pc - root_candidate + 12) % 12;

                if (interval == 0) has_root = true;

                // Add interval if not already present
                bool already_present = false;
                for (int j = 0; j < interval_count; ++j) {
                    if (intervals[j] == interval) {
                        already_present = true;
                        break;
                    }
                }
                if (!already_present && interval_count < MAX_NOTES) {
                    intervals[interval_count++] = interval;
                }
            }

            if (!has_root || interval_count < 2) continue;

            insertion_sort(intervals, interval_count);
            uint16_t mask = create_interval_mask(intervals, interval_count);

            // Find best pattern match for this root
            for (size_t p = 0; p < NUM_PATTERNS; ++p) {
                const ChordPattern& pattern = CHORD_PATTERNS[p];

                if (pattern.mask == mask && pattern.priority > best_priority) {
                    std::string root_name = note_names[root_candidate];
                    std::string bass_name = note_names[bass_pitch_class];

                    // Special handling for ambiguous patterns
                    if (strcmp(pattern.name, "?") == 0) {
                        // For C-D-F (intervals 0,2,5 from C), this should be Dm/C
                        if (mask == ((1<<0)|(1<<2)|(1<<5))) { // M2 + P4 from bass
                            // Check if it forms a minor chord from the second note
                            int second_note = (bass_pitch_class + 2) % 12; // D if bass is C
                            bool forms_minor = true;

                            // Verify D-F (minor third) exists
                            for (int i = 0; i < note_count; ++i) {
                                int pc = midi_notes[i] % 12;
                                if (pc == (second_note + 3) % 12) { // F if second_note is D
                                    best_result = {
                                        std::string(note_names[second_note]) + "m(omit5)/" + bass_name,
                                        std::string(note_names[second_note]) + "m(omit5)",
                                        bass_name,
                                        true,
                                        second_note,
                                        bass_pitch_class
                                    };
                                    best_priority = pattern.priority + 10; // Higher priority for this special case
                                    break;
                                }
                            }
                        }
                        continue;
                    }

                    best_result = {
                        root_name + pattern.name + "/" + bass_name,
                        root_name + pattern.name,
                        bass_name,
                        true,
                        root_candidate,
                        bass_pitch_class
                    };
                    best_priority = pattern.priority;
                }
            }
        }

        return best_result;
    }
}

// Main chord analysis function - enhanced for omit5 and add11
ChordResult analyze_chord(const int* midi_notes, int note_count, bool use_flats = false, bool use_slash = false) {
    ChordResult result = {"", "", "", false, -1, -1};

    if (note_count <= 0) return result;

    const char* const* note_names = use_flats ?
        ChordDetector::NOTE_NAMES_FLAT : ChordDetector::NOTE_NAMES_SHARP;

    // Find bass note (lowest MIDI note for slash chord detection)
    int bass_midi = midi_notes[0];
    for (int i = 1; i < note_count; ++i) {
        if (midi_notes[i] < bass_midi) {
            bass_midi = midi_notes[i];
        }
    }
    int bass_pitch_class = bass_midi % 12;

    // Try each note as root
    int best_priority = -1;
    ChordResult best_slash_result = {"", "", "", false, -1, -1};

    for (int root_candidate = 0; root_candidate < 12; ++root_candidate) {
        // Build intervals from this root
        int intervals[ChordDetector::MAX_NOTES];
        int interval_count = 0;
        bool has_root = false;

        for (int i = 0; i < note_count; ++i) {
            int midi = midi_notes[i];
            if (midi < 0 || midi > 127) continue;

            int pc = midi % 12;
            int interval = (pc - root_candidate + 12) % 12;

            if (interval == 0) has_root = true;

            // Add interval if not already present
            bool already_present = false;
            for (int j = 0; j < interval_count; ++j) {
                if (intervals[j] == interval) {
                    already_present = true;
                    break;
                }
            }
            if (!already_present && interval_count < ChordDetector::MAX_NOTES) {
                intervals[interval_count++] = interval;
            }
        }

        if (!has_root || interval_count < 2) continue;

        ChordDetector::insertion_sort(intervals, interval_count);
        uint16_t mask = ChordDetector::create_interval_mask(intervals, interval_count);

        // Find best pattern match
        for (size_t p = 0; p < ChordDetector::NUM_PATTERNS; ++p) {
            const ChordDetector::ChordPattern& pattern = ChordDetector::CHORD_PATTERNS[p];

            if (pattern.mask != mask) continue;

            int priority = pattern.priority;

            // Bonus for root position (root = bass)
            if (root_candidate == bass_pitch_class) {
                priority += 30;
            }

            if (priority > best_priority) {
                std::string root_name = note_names[root_candidate];
                std::string bass_name = note_names[bass_pitch_class];

                bool is_slash = (root_candidate != bass_pitch_class) && use_slash;

                result = {
                    is_slash ? root_name + pattern.name + "/" + bass_name : root_name + pattern.name,
                    root_name + pattern.name,
                    bass_name,
                    is_slash,
                    root_candidate,
                    bass_pitch_class
                };
                best_priority = priority;
            }
        }
    }

    // Enhanced slash chord analysis for special cases
    if (use_slash && best_priority < 50) { // If no good root position match found
        ChordResult slash_result = ChordDetector::analyze_for_slash_chord(
            midi_notes, note_count, bass_pitch_class, note_names);

        if (!slash_result.full_name.empty()) {
            result = slash_result;
        }
    }

    return result;
}

// Simple chord name function
std::string get_chord_name(const int* midi_notes, int note_count, bool use_flats = false, bool use_slash = false) {
    return analyze_chord(midi_notes, note_count, use_flats, use_slash).full_name;
}

// Convenience overloads
ChordResult analyze_chord(const std::vector<int>& midi_notes, bool use_flats = false, bool use_slash = false) {
    return analyze_chord(midi_notes.data(), static_cast<int>(midi_notes.size()), use_flats, use_slash);
}

ChordResult analyze_chord(std::initializer_list<int> midi_notes, bool use_flats = false, bool use_slash = false) {
    return analyze_chord(midi_notes.begin(), static_cast<int>(midi_notes.size()), use_flats, use_slash);
}

template<size_t N>
ChordResult analyze_chord(const std::array<int, N>& midi_notes, bool use_flats = false, bool use_slash = false) {
    return analyze_chord(midi_notes.data(), static_cast<int>(N), use_flats, use_slash);
}

template<size_t N>
ChordResult analyze_chord(const int (&midi_notes)[N], bool use_flats = false, bool use_slash = false) {
    return analyze_chord(midi_notes, static_cast<int>(N), use_flats, use_slash);
}

std::string get_chord_name(const std::vector<int>& midi_notes, bool use_flats = false, bool use_slash = false) {
    return get_chord_name(midi_notes.data(), static_cast<int>(midi_notes.size()), use_flats, use_slash);
}

std::string get_chord_name(std::initializer_list<int> midi_notes, bool use_flats = false, bool use_slash = false) {
    return get_chord_name(midi_notes.begin(), static_cast<int>(midi_notes.size()), use_flats, use_slash);
}

template<size_t N>
std::string get_chord_name(const std::array<int, N>& midi_notes, bool use_flats = false, bool use_slash = false) {
    return get_chord_name(midi_notes.data(), static_cast<int>(N), use_flats, use_slash);
}

template<size_t N>
std::string get_chord_name(const int (&midi_notes)[N], bool use_flats = false, bool use_slash = false) {
    return get_chord_name(midi_notes, static_cast<int>(N), use_flats, use_slash);
}

// Legacy compatibility functions
std::string get_slash_chord_name(const int* midi_notes, int note_count, bool use_flats = false) {
    return get_chord_name(midi_notes, note_count, use_flats, true);
}

std::string get_slash_chord_name(const std::vector<int>& midi_notes, bool use_flats = false) {
    return get_chord_name(midi_notes, use_flats, true);
}

std::string get_slash_chord_name(std::initializer_list<int> midi_notes, bool use_flats = false) {
    return get_chord_name(midi_notes, use_flats, true);
}

template<size_t N>
std::string get_slash_chord_name(const std::array<int, N>& midi_notes, bool use_flats = false) {
    return get_chord_name(midi_notes, use_flats, true);
}

template<size_t N>
std::string get_slash_chord_name(const int (&midi_notes)[N], bool use_flats = false) {
    return get_chord_name(midi_notes, use_flats, true);
}

ChordResult analyze_slash_chord(const int* midi_notes, int note_count, bool use_flats = false) {
    return analyze_chord(midi_notes, note_count, use_flats, true);
}

ChordResult analyze_slash_chord(const std::vector<int>& midi_notes, bool use_flats = false) {
    return analyze_chord(midi_notes, use_flats, true);
}

ChordResult analyze_slash_chord(std::initializer_list<int> midi_notes, bool use_flats = false) {
    return analyze_chord(midi_notes, use_flats, true);
}

template<size_t N>
ChordResult analyze_slash_chord(const std::array<int, N>& midi_notes, bool use_flats = false) {
    return analyze_chord(midi_notes, use_flats, true);
}

template<size_t N>
ChordResult analyze_slash_chord(const int (&midi_notes)[N], bool use_flats = false) {
    return analyze_chord(midi_notes, use_flats, true);
}

// Inversion type detection helper
inline std::string get_inversion_type(const ChordResult& chord) {
    if (!chord.is_slash_chord) return "root";

    int interval = (chord.bass_pitch_class - chord.root_pitch_class + 12) % 12;
    switch (interval) {
        case 0: return "root";
        case 3:
        case 4: return "1st";  // 3rd in bass
        case 6:
        case 7: return "2nd";  // 5th in bass
        case 10:
        case 11: return "3rd"; // 7th in bass
        default: return "other";
    }
}

// Detailed analysis helper
struct DetailedAnalysis {
    ChordResult chord;
    std::string inversion_type;
    std::vector<std::string> note_names;
    std::vector<int> intervals_from_root;
};

DetailedAnalysis get_detailed_analysis(const int* midi_notes, int note_count, bool use_flats = false) {
    DetailedAnalysis analysis;
    analysis.chord = analyze_chord(midi_notes, note_count, use_flats, true);
    analysis.inversion_type = get_inversion_type(analysis.chord);

    const char* const* note_names = use_flats ?
        ChordDetector::NOTE_NAMES_FLAT : ChordDetector::NOTE_NAMES_SHARP;

    // Get all unique pitch classes
    bool active[12] = {false};
    for (int i = 0; i < note_count; ++i) {
        int midi = midi_notes[i];
        if (midi >= 0 && midi <= 127) {
            active[midi % 12] = true;
        }
    }

    for (int i = 0; i < 12; ++i) {
        if (active[i]) {
            analysis.note_names.push_back(note_names[i]);
            if (analysis.chord.root_pitch_class >= 0) {
                int interval = (i - analysis.chord.root_pitch_class + 12) % 12;
                analysis.intervals_from_root.push_back(interval);
            }
        }
    }

    return analysis;
}

// Convenience overloads for detailed analysis
DetailedAnalysis get_detailed_analysis(const std::vector<int>& midi_notes, bool use_flats = false) {
    return get_detailed_analysis(midi_notes.data(), static_cast<int>(midi_notes.size()), use_flats);
}

DetailedAnalysis get_detailed_analysis(std::initializer_list<int> midi_notes, bool use_flats = false) {
    return get_detailed_analysis(midi_notes.begin(), static_cast<int>(midi_notes.size()), use_flats);
}

template<size_t N>
DetailedAnalysis get_detailed_analysis(const std::array<int, N>& midi_notes, bool use_flats = false) {
    return get_detailed_analysis(midi_notes.data(), static_cast<int>(N), use_flats);
}

template<size_t N>
DetailedAnalysis get_detailed_analysis(const int (&midi_notes)[N], bool use_flats = false) {
    return get_detailed_analysis(midi_notes, static_cast<int>(N), use_flats);
}
