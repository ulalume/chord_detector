#include "chord_detector.h"
#include <iostream>

int main() {
    std::cout << "Chord Detector Example\n";
    std::cout << "======================\n\n";

    // Basic chord detection
    std::cout << "Basic chords:\n";
    std::cout << "C Major:  " << get_chord_name({60, 64, 67}) << "\n";
    std::cout << "A Minor:  " << get_chord_name({69, 72, 76}) << "\n";
    std::cout << "G7:       " << get_chord_name({67, 71, 74, 77}) << "\n\n";

    // Slash chords
    std::cout << "Slash chords:\n";
    std::cout << "C/E:      " << get_chord_name({64, 67, 72}, false, true) << "\n";
    std::cout << "Am/C:     " << get_chord_name({72, 76, 81}, false, true) << "\n";
    std::cout << "G7/B:     " << get_chord_name({71, 74, 77, 79}, false, true) << "\n\n";

    // Extended chords
    std::cout << "Extended chords:\n";
    std::cout << "Cmaj9:    " << get_chord_name({60, 64, 67, 71, 74}) << "\n";
    std::cout << "Am11:     " << get_chord_name({69, 74, 76, 79, 81, 84}) << "\n\n";

    // Detailed analysis
    ChordResult result = analyze_chord({71, 74, 77, 79}, false, true);
    std::cout << "Analysis of B-D-F-G:\n";
    std::cout << "Chord: " << result.full_name << "\n";
    std::cout << "Bass:  " << result.bass_note << "\n";
    std::cout << "Slash: " << (result.is_slash_chord ? "Yes" : "No") << "\n";

    return 0;
}
