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

    // Pre-computed chord patterns sorted by priority (higher first)
    constexpr ChordPattern CHORD_PATTERNS[] = {
        // Extended chords (higher priority due to more specificity)
        {(1<<2)|(1<<4)|(1<<5)|(1<<7)|(1<<10), "11",     100},  // Dom11: M2,M3,P4,P5,m7
        {(1<<2)|(1<<4)|(1<<5)|(1<<7)|(1<<11), "maj11",  100},  // Maj11: M2,M3,P4,P5,M7
        {(1<<2)|(1<<3)|(1<<5)|(1<<7)|(1<<10), "m11",    100},  // Min11: M2,m3,P4,P5,m7
        {(1<<2)|(1<<4)|(1<<7)|(1<<10), "9",      90},   // Dom9: M2,M3,P5,m7
        {(1<<2)|(1<<4)|(1<<7)|(1<<11), "maj9",   90},   // Maj9: M2,M3,P5,M7
        {(1<<2)|(1<<3)|(1<<7)|(1<<10), "m9",     90},   // Min9: M2,m3,P5,m7
        {(1<<2)|(1<<3)|(1<<7)|(1<<11), "mM9",    90},   // MinMaj9: M2,m3,P5,M7

        // 7th chords
        {(1<<4)|(1<<7)|(1<<10), "7",      85},   // Dom7: M3,P5,m7
        {(1<<4)|(1<<7)|(1<<11), "maj7",   85},   // Maj7: M3,P5,M7
        {(1<<3)|(1<<7)|(1<<10), "m7",     85},   // Min7: m3,P5,m7
        {(1<<3)|(1<<7)|(1<<11), "mM7",    85},   // MinMaj7: m3,P5,M7
        {(1<<4)|(1<<6)|(1<<10), "7b5",    80},   // 7b5: M3,b5,m7
        {(1<<3)|(1<<6)|(1<<10), "m7b5",   80},   // HalfDim: m3,b5,m7
        {(1<<3)|(1<<6)|(1<<9), "o7",     80},   // Dim7: m3,b5,d7
        {(1<<5)|(1<<7)|(1<<10), "7sus4",  75},   // 7sus4: P4,P5,m7

        // 6th chords
        {(1<<4)|(1<<7)|(1<<9), "6",      82},   // Maj6: M3,P5,M6
        {(1<<3)|(1<<7)|(1<<9), "m6",     82},   // Min6: m3,P5,M6

        // Add chords
        {(1<<2)|(1<<4)|(1<<7), "add9",   60},   // Add9: M2,M3,P5
        {(1<<2)|(1<<3)|(1<<7), "madd9",  60},   // MinAdd9: M2,m3,P5

        // Basic triads
        {(1<<4)|(1<<7), "",       50},   // Major: M3,P5
        {(1<<3)|(1<<7), "m",      50},   // Minor: m3,P5
        {(1<<4)|(1<<8), "+",      45},   // Aug: M3,m6
        {(1<<3)|(1<<6), "o",      45},   // Dim: m3,b5
        {(1<<2)|(1<<7), "sus2",   40},   // Sus2: M2,P5
        {(1<<5)|(1<<7), "sus4",   40},   // Sus4: P4,P5

        // Power chords and incomplete
        {(1<<7), "5",      30},   // Power: P5
        {(1<<4), "",       20},   // Just M3
        {(1<<3), "m",      20},   // Just m3
    };

    constexpr size_t NUM_PATTERNS = sizeof(CHORD_PATTERNS) / sizeof(ChordPattern);

    // Note name lookup tables
    constexpr const char* NOTE_NAMES_SHARP[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

    constexpr const char* NOTE_NAMES_FLAT[] = {
        "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
    };

    // Fast interval mask calculation
    inline uint16_t calculate_interval_mask(const int* intervals, int count) {
        uint16_t mask = 0;
        for (int i = 0; i < count; ++i) {
            if (intervals[i] > 0 && intervals[i] < 12) {
                mask |= (1u << intervals[i]);
            }
        }
        return mask;
    }
}

// Main chord analysis function - optimized for performance
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
    int bass_pc = bass_midi % 12;

    result.bass_note = note_names[bass_pc];
    result.bass_pitch_class = bass_pc;

    // Convert to pitch classes using fixed array (stack allocation)
    bool active_notes[12] = {false};
    std::array<int, ChordDetector::MAX_NOTES> pitch_classes;
    int pc_count = 0;

    // Collect unique pitch classes
    for (int i = 0; i < note_count; ++i) {
        int midi = midi_notes[i];
        if (midi >= 0 && midi <= 127) {
            int pc = midi % 12;
            if (!active_notes[pc]) {
                active_notes[pc] = true;
                if (pc_count < ChordDetector::MAX_NOTES) {
                    pitch_classes[pc_count++] = pc;
                }
            }
        }
    }

    if (pc_count < 1) return result;
    if (pc_count == 1) {
        result.chord_name = note_names[pitch_classes[0]];
        result.full_name = result.chord_name;
        result.root_pitch_class = pitch_classes[0];
        result.is_slash_chord = false;
        return result;
    }

    // Try each pitch class as root
    const char* best_chord_name = nullptr;
    int best_root = -1;
    int best_priority = -1;

    // Pre-allocate interval array for reuse
    std::array<int, ChordDetector::MAX_NOTES - 1> intervals;

    for (int root_idx = 0; root_idx < pc_count; ++root_idx) {
        int root = pitch_classes[root_idx];
        int interval_count = 0;

        // Calculate intervals from root
        for (int i = 0; i < pc_count; ++i) {
            if (i != root_idx) {
                int interval = (pitch_classes[i] - root + 12) % 12;
                if (interval > 0 && interval_count < ChordDetector::MAX_NOTES - 1) {
                    intervals[interval_count++] = interval;
                }
            }
        }

        if (interval_count == 0) continue;

        // Sort intervals using insertion sort (optimal for small arrays)
        for (int i = 1; i < interval_count; ++i) {
            int key = intervals[i];
            int j = i - 1;
            while (j >= 0 && intervals[j] > key) {
                intervals[j + 1] = intervals[j];
                --j;
            }
            intervals[j + 1] = key;
        }

        // Calculate bitmask for O(1) pattern lookup
        uint16_t mask = ChordDetector::calculate_interval_mask(intervals.data(), interval_count);

        // Find matching pattern
        for (size_t p = 0; p < ChordDetector::NUM_PATTERNS; ++p) {
            if (ChordDetector::CHORD_PATTERNS[p].mask == mask) {
                int priority = ChordDetector::CHORD_PATTERNS[p].priority;

                // Bonus for root position (bass note = root)
                if (root == bass_pc) {
                    priority += 20;
                }

                if (priority > best_priority) {
                    best_chord_name = ChordDetector::CHORD_PATTERNS[p].name;
                    best_root = root;
                    best_priority = priority;
                }
                break;
            }
        }
    }

    // Build result
    if (best_root >= 0) {
        result.chord_name = std::string(note_names[best_root]) + (best_chord_name ? best_chord_name : "");
        result.root_pitch_class = best_root;

        if (use_slash && best_root != bass_pc) {
            // Slash chord notation
            result.full_name = result.chord_name + "/" + result.bass_note;
            result.is_slash_chord = true;
        } else {
            // Root position or slash chords disabled
            result.full_name = result.chord_name;
            result.is_slash_chord = false;
        }
    } else {
        // Fallback to bass note
        result.chord_name = result.bass_note;
        result.full_name = result.chord_name;
        result.root_pitch_class = bass_pc;
        result.is_slash_chord = false;
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

// Simple chord name overloads
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

// Legacy alias functions for backward compatibility
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

// Inversion analysis helper functions
std::string get_inversion_type(const ChordResult& result) {
    if (!result.is_slash_chord) {
        return "root";
    }

    int interval = (result.bass_pitch_class - result.root_pitch_class + 12) % 12;

    switch (interval) {
        case 3:
        case 4: return "1st";  // 3rd in bass
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
