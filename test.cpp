#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cassert>
#include <chrono>
#include "chord_detector.h"

// Test result tracking
struct TestResult {
    int passed = 0;
    int failed = 0;

    void assert_equal(const std::string& expected, const std::string& actual, const std::string& test_name) {
        if (expected == actual) {
            std::cout << "✓ " << test_name << ": " << actual << std::endl;
            passed++;
        } else {
            std::cout << "✗ " << test_name << ": expected '" << expected << "', got '" << actual << "'" << std::endl;
            failed++;
        }
    }

    void assert_bool(bool expected, bool actual, const std::string& test_name) {
        if (expected == actual) {
            std::cout << "✓ " << test_name << ": " << (actual ? "true" : "false") << std::endl;
            passed++;
        } else {
            std::cout << "✗ " << test_name << ": expected " << (expected ? "true" : "false")
                     << ", got " << (actual ? "true" : "false") << std::endl;
            failed++;
        }
    }

    void print_summary() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "Test Summary: " << passed << " passed, " << failed << " failed" << std::endl;
        if (failed == 0) {
            std::cout << "🎉 All tests passed!" << std::endl;
        } else {
            std::cout << "❌ " << failed << " test(s) failed" << std::endl;
        }
        std::cout << std::string(60, '=') << std::endl;
    }
};

TestResult result;

void test_basic_chord_detection() {
    std::cout << "\n--- Basic Chord Detection (use_slash=false) ---" << std::endl;

    // Basic triads - should not show slash notation even for inversions
    result.assert_equal("C", get_chord_name({60, 64, 67}), "C Major root");
    result.assert_equal("C", get_chord_name({64, 67, 72}), "C Major 1st inv (no slash)");
    result.assert_equal("C", get_chord_name({67, 72, 76}), "C Major 2nd inv (no slash)");

    result.assert_equal("Am", get_chord_name({69, 72, 76}), "A Minor root");
    result.assert_equal("Am", get_chord_name({72, 76, 81}), "A Minor 1st inv (no slash)");

    // 7th chords
    result.assert_equal("G7", get_chord_name({67, 71, 74, 77}), "G7 root");
    result.assert_equal("G7", get_chord_name({71, 74, 77, 79}), "G7 1st inv (no slash)");
    result.assert_equal("Cmaj7", get_chord_name({60, 64, 67, 71}), "Cmaj7 root");

    // Extended chords
    result.assert_equal("C9", get_chord_name({60, 64, 67, 70, 74}), "C9");
    result.assert_equal("Cmaj9", get_chord_name({60, 64, 67, 71, 74}), "Cmaj9");
    result.assert_equal("C11", get_chord_name({60, 62, 64, 65, 67, 70}), "C11");
}

void test_slash_chord_detection() {
    std::cout << "\n--- Slash Chord Detection (use_slash=true) ---" << std::endl;

    // Basic triads with slash notation
    result.assert_equal("C", get_chord_name({60, 64, 67}, false, true), "C Major root (slash enabled)");
    result.assert_equal("C/E", get_chord_name({64, 67, 72}, false, true), "C/E (1st inversion)");
    result.assert_equal("C/G", get_chord_name({67, 72, 76}, false, true), "C/G (2nd inversion)");

    result.assert_equal("Am", get_chord_name({69, 72, 76}, false, true), "Am root (slash enabled)");
    result.assert_equal("Am/C", get_chord_name({72, 76, 81}, false, true), "Am/C (1st inversion)");
    result.assert_equal("Am/E", get_chord_name({76, 81, 84}, false, true), "Am/E (2nd inversion)");

    // 7th chords with slash notation
    result.assert_equal("G7", get_chord_name({67, 71, 74, 77}, false, true), "G7 root (slash enabled)");
    result.assert_equal("G7/B", get_chord_name({71, 74, 77, 79}, false, true), "G7/B (1st inversion)");
    result.assert_equal("G7/D", get_chord_name({74, 77, 79, 83}, false, true), "G7/D (2nd inversion)");
    result.assert_equal("G7/F", get_chord_name({77, 79, 83, 86}, false, true), "G7/F (3rd inversion)");

    // Extended chords with slash notation
    result.assert_equal("C9/E", get_chord_name({64, 67, 70, 72, 74}, false, true), "C9/E");
    result.assert_equal("Cmaj9/E", get_chord_name({64, 67, 71, 72, 74}, false, true), "Cmaj9/E");
}

void test_analyze_chord_function() {
    std::cout << "\n--- ChordResult Analysis ---" << std::endl;

    // Root position chord
    ChordResult c_root = analyze_chord({60, 64, 67});
    result.assert_equal("C", c_root.full_name, "C root full_name");
    result.assert_equal("C", c_root.chord_name, "C root chord_name");
    result.assert_equal("C", c_root.bass_note, "C root bass_note");
    result.assert_bool(false, c_root.is_slash_chord, "C root not slash");

    // First inversion without slash
    ChordResult c_e_no_slash = analyze_chord({64, 67, 72});
    result.assert_equal("C", c_e_no_slash.full_name, "C/E no slash full_name");
    result.assert_equal("C", c_e_no_slash.chord_name, "C/E no slash chord_name");
    result.assert_equal("E", c_e_no_slash.bass_note, "C/E no slash bass_note");
    result.assert_bool(false, c_e_no_slash.is_slash_chord, "C/E no slash not slash");

    // First inversion with slash
    ChordResult c_e_slash = analyze_chord({64, 67, 72}, false, true);
    result.assert_equal("C/E", c_e_slash.full_name, "C/E slash full_name");
    result.assert_equal("C", c_e_slash.chord_name, "C/E slash chord_name");
    result.assert_equal("E", c_e_slash.bass_note, "C/E slash bass_note");
    result.assert_bool(true, c_e_slash.is_slash_chord, "C/E slash is slash");

    // 7th chord inversion
    ChordResult g7_b = analyze_chord({71, 74, 77, 79}, false, true);
    result.assert_equal("G7/B", g7_b.full_name, "G7/B full_name");
    result.assert_equal("G7", g7_b.chord_name, "G7/B chord_name");
    result.assert_equal("B", g7_b.bass_note, "G7/B bass_note");
    result.assert_bool(true, g7_b.is_slash_chord, "G7/B is slash");
}

void test_legacy_compatibility() {
    std::cout << "\n--- Legacy Function Compatibility ---" << std::endl;

    // Test legacy get_slash_chord_name functions
    result.assert_equal("C/E", get_slash_chord_name({64, 67, 72}), "Legacy get_slash_chord_name");
    result.assert_equal("G7/B", get_slash_chord_name({71, 74, 77, 79}), "Legacy G7/B");

    // Test legacy analyze_slash_chord functions
    ChordResult legacy_result = analyze_slash_chord({64, 67, 72});
    result.assert_equal("C/E", legacy_result.full_name, "Legacy analyze_slash_chord");
    result.assert_bool(true, legacy_result.is_slash_chord, "Legacy is_slash_chord");
}

void test_inversion_analysis() {
    std::cout << "\n--- Inversion Type Analysis ---" << std::endl;

    ChordResult root = analyze_chord({60, 64, 67}, false, true);
    result.assert_equal("root", get_inversion_type(root), "Root position type");

    ChordResult first = analyze_chord({64, 67, 72}, false, true);
    result.assert_equal("1st", get_inversion_type(first), "First inversion type");

    ChordResult second = analyze_chord({67, 72, 76}, false, true);
    result.assert_equal("2nd", get_inversion_type(second), "Second inversion type");

    ChordResult third = analyze_chord({77, 79, 83, 86}, false, true); // G7/F
    result.assert_equal("3rd", get_inversion_type(third), "Third inversion type");

    ChordResult other = analyze_chord({62, 67, 72, 76}, false, true); // Cadd9/D
    result.assert_equal("other", get_inversion_type(other), "Other bass note type");
}

void test_detailed_analysis() {
    std::cout << "\n--- Detailed Analysis ---" << std::endl;

    DetailedAnalysis analysis = get_detailed_analysis({71, 74, 77, 79}); // G7/B

    result.assert_equal("G7/B", analysis.chord.full_name, "Detailed analysis chord name");
    result.assert_equal("1st", analysis.inversion_type, "Detailed analysis inversion");

    // Check that all expected notes are present
    bool has_b = false, has_d = false, has_f = false, has_g = false;
    for (const auto& note : analysis.note_names) {
        if (note == "B") has_b = true;
        if (note == "D") has_d = true;
        if (note == "F") has_f = true;
        if (note == "G") has_g = true;
    }
    result.assert_bool(true, has_b && has_d && has_f && has_g, "Detailed analysis has all notes");

    // Check intervals from root (G = 0)
    bool has_intervals = analysis.intervals_from_root.size() >= 4;
    result.assert_bool(true, has_intervals, "Detailed analysis has intervals");
}

void test_sharp_flat_notation() {
    std::cout << "\n--- Sharp/Flat Notation ---" << std::endl;

    // Sharp notation (default)
    result.assert_equal("C#", get_chord_name({61, 65, 68}), "C# Major (sharp)");
    result.assert_equal("C#/F", get_chord_name({65, 68, 73}, false, true), "C#/F (sharp slash)");

    // Flat notation
    result.assert_equal("Db", get_chord_name({61, 65, 68}, true), "Db Major (flat)");
    result.assert_equal("Db/F", get_chord_name({65, 68, 73}, true, true), "Db/F (flat slash)");

    // Complex chords
    result.assert_equal("F#7", get_chord_name({66, 70, 73, 76}), "F#7 (sharp)");
    result.assert_equal("Gb7", get_chord_name({66, 70, 73, 76}, true), "Gb7 (flat)");
    result.assert_equal("F#7/A#", get_chord_name({70, 73, 76, 78}, false, true), "F#7/A# (sharp slash)");
    result.assert_equal("Gb7/Bb", get_chord_name({70, 73, 76, 78}, true, true), "Gb7/Bb (flat slash)");
}

void test_different_input_formats() {
    std::cout << "\n--- Different Input Formats ---" << std::endl;

    // C array
    int c_array[] = {64, 67, 72};
    result.assert_equal("C", get_chord_name(c_array), "C array basic");
    result.assert_equal("C/E", get_chord_name(c_array, false, true), "C array slash");

    // std::vector
    std::vector<int> vector_input = {64, 67, 72};
    result.assert_equal("C", get_chord_name(vector_input), "Vector basic");
    result.assert_equal("C/E", get_chord_name(vector_input, false, true), "Vector slash");

    // std::array
    std::array<int, 3> array_input = {64, 67, 72};
    result.assert_equal("C", get_chord_name(array_input), "Array basic");
    result.assert_equal("C/E", get_chord_name(array_input, false, true), "Array slash");

    // Initializer list
    result.assert_equal("C", get_chord_name({64, 67, 72}), "Init list basic");
    result.assert_equal("C/E", get_chord_name({64, 67, 72}, false, true), "Init list slash");

    // Raw pointer + count
    int raw_data[] = {64, 67, 72, 76};
    result.assert_equal("C", get_chord_name(static_cast<const int*>(raw_data), 4), "Raw pointer basic");
    result.assert_equal("C/E", get_chord_name(static_cast<const int*>(raw_data), 4, false, true), "Raw pointer slash");
}

void test_edge_cases() {
    std::cout << "\n--- Edge Cases ---" << std::endl;

    // Empty input
    ChordResult empty = analyze_chord(static_cast<int*>(nullptr), 0);
    result.assert_equal("", empty.full_name, "Empty input");

    // Single note
    ChordResult single = analyze_chord({60});
    result.assert_equal("", single.full_name, "Single note");
    result.assert_bool(false, single.is_slash_chord, "Single note not slash");

    // Single note with slash enabled (should still not be slash)
    ChordResult single_slash = analyze_chord({60}, false, true);
    result.assert_equal("", single_slash.full_name, "Single note slash enabled");
    result.assert_bool(false, single_slash.is_slash_chord, "Single note slash enabled not slash");

    // Invalid MIDI values (should be filtered)
    ChordResult invalid = analyze_chord({-1, 60, 64, 67, 128});
    bool valid_result = !invalid.full_name.empty() && invalid.full_name.substr(0, 1) == "C";
    result.assert_bool(true, valid_result, "Invalid MIDI filtered");

    // Duplicate notes
    result.assert_equal("C", get_chord_name({60, 60, 64, 64, 67, 67}), "Duplicate notes basic");
    result.assert_equal("C/E", get_chord_name({64, 64, 67, 67, 72, 72}, false, true), "Duplicate notes slash");

    // Octave doubling
    result.assert_equal("C", get_chord_name({48, 60, 64, 67, 72}), "Octave doubling basic");
    result.assert_equal("C/E", get_chord_name({52, 64, 67, 72, 76}, false, true), "Octave doubling slash");
}

void test_musical_equivalents() {
    std::cout << "\n--- Musical Equivalents ---" << std::endl;

    // Test cases where different interpretations are musically equivalent
    // C6 vs Am7 (same notes: C-E-G-A vs A-C-E-G)
    result.assert_equal("C6", get_chord_name({60, 64, 67, 69}), "C6 root position");
    result.assert_equal("Am7", get_chord_name({69, 72, 76, 79}), "Am7 root position");

    // With slash notation, bass note determines interpretation
    result.assert_equal("C6", get_chord_name({60, 64, 67, 69}, false, true), "C6 slash (root)");
    result.assert_equal("Am7", get_chord_name({69, 72, 76, 79}, false, true), "Am7 slash (root)");
    result.assert_equal("C6", get_chord_name({72, 76, 79, 81}, false, true), "C6 (Am7/C equivalent)");
    result.assert_equal("Am7", get_chord_name({69, 72, 76, 79}, false, true), "Am7 (C6/A equivalent)");
}

void test_performance() {
    std::cout << "\n--- Performance Test ---" << std::endl;

    const int iterations = 100000;
    std::vector<int> test_chord = {71, 74, 77, 79}; // G7/B

    // Test basic chord detection performance
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        volatile std::string result_basic = get_chord_name(test_chord);
        (void)result_basic;
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);

    // Test slash chord detection performance
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        volatile std::string result_slash = get_chord_name(test_chord, false, true);
        (void)result_slash;
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);

    // Test full analysis performance
    auto start3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        volatile ChordResult result_analysis = analyze_chord(test_chord, false, true);
        (void)result_analysis;
    }
    auto end3 = std::chrono::high_resolution_clock::now();
    auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(end3 - start3);

    double avg_basic = static_cast<double>(duration1.count()) / iterations;
    double avg_slash = static_cast<double>(duration2.count()) / iterations;
    double avg_analysis = static_cast<double>(duration3.count()) / iterations;

    std::cout << "Basic chord detection: " << avg_basic << " μs/call" << std::endl;
    std::cout << "Slash chord detection: " << avg_slash << " μs/call" << std::endl;
    std::cout << "Full chord analysis: " << avg_analysis << " μs/call" << std::endl;

    // All should be under 2 μs for real-time usage
    if (avg_basic < 2.0 && avg_slash < 2.0 && avg_analysis < 2.0) {
        std::cout << "✓ Performance: All functions suitable for real-time usage" << std::endl;
        result.passed++;
    } else {
        std::cout << "✗ Performance: Some functions too slow for real-time usage" << std::endl;
        result.failed++;
    }
}

void test_common_progressions() {
    std::cout << "\n--- Common Progressions ---" << std::endl;

    // I-vi-IV-V in C Major (basic)
    result.assert_equal("C", get_chord_name({60, 64, 67}), "I (C) basic");
    result.assert_equal("Am", get_chord_name({69, 72, 76}), "vi (Am) basic");
    result.assert_equal("F", get_chord_name({65, 69, 72}), "IV (F) basic");
    result.assert_equal("G", get_chord_name({67, 71, 74}), "V (G) basic");

    // Same progression with bass lines (slash chords)
    result.assert_equal("C", get_chord_name({60, 64, 67}, false, true), "I (C) slash");
    result.assert_equal("C/E", get_chord_name({64, 67, 72}, false, true), "I/3 (C/E) slash");
    result.assert_equal("Am", get_chord_name({69, 72, 76}, false, true), "vi (Am) slash");
    result.assert_equal("Am/C", get_chord_name({72, 76, 81}, false, true), "vi/♭3 (Am/C) slash");

    // Jazz ii-V-I with 7th chords
    result.assert_equal("Dm7", get_chord_name({62, 65, 69, 72}), "ii7 (Dm7) basic");
    result.assert_equal("G7", get_chord_name({67, 71, 74, 77}), "V7 (G7) basic");
    result.assert_equal("Cmaj7", get_chord_name({60, 64, 67, 71}), "Imaj7 (Cmaj7) basic");

    // Same with inversions
    result.assert_equal("F6", get_chord_name({65, 69, 72, 74}, false, true), "F6 (Dm7/F equivalent) slash");
    result.assert_equal("G7/B", get_chord_name({71, 74, 77, 79}, false, true), "V7/3 (G7/B) slash");
    result.assert_equal("Cmaj7", get_chord_name({60, 64, 67, 71}, false, true), "Imaj7 (Cmaj7) slash");
}

void test_omit5_and_add11_patterns() {
    std::cout << "\n--- Enhanced omit5 and add11 Pattern Tests ---" << std::endl;
    
    // Test case 1: C-E-F (ド、ミ、ファ) - should be Cadd11(omit5)
    result.assert_equal("Cadd11(omit5)", get_chord_name({60, 64, 65}), "C-E-F (Cadd11(omit5))");
    
    // Test case 2: C-D-F (ド、レ、ファ) - should be detected as slash chord
    result.assert_equal("Dm7(omit5)/C", get_chord_name({60, 62, 65}, false, true), "C-D-F slash chord");
    
    // Additional omit5 tests
    result.assert_equal("C7(omit5)", get_chord_name({60, 64, 70}), "C-E-Bb (C7 omit5)");
    result.assert_equal("Cmaj7(omit5)", get_chord_name({60, 64, 71}), "C-E-B (Cmaj7 omit5)");
    result.assert_equal("Cm7(omit5)", get_chord_name({60, 63, 70}), "C-Eb-Bb (Cm7 omit5)");
    
    // Test add11 with 5th present
    result.assert_equal("Cadd11", get_chord_name({60, 64, 65, 67}), "C-E-F-G (Cadd11)");
    result.assert_equal("Dmadd11", get_chord_name({62, 65, 67, 69}), "D-F-G-A (Dmadd11)");
    
    // Test 9th omit5 chords
    result.assert_equal("C9(omit5)", get_chord_name({60, 62, 64, 70}), "C-D-E-Bb (C9 omit5)");
    result.assert_equal("Cmaj9(omit5)", get_chord_name({60, 62, 64, 71}), "C-D-E-B (Cmaj9 omit5)");
    
    // Test 6th omit5 chords
    result.assert_equal("C6(omit5)", get_chord_name({60, 64, 69}), "C-E-A (C6 omit5)");
    result.assert_equal("Cm6(omit5)", get_chord_name({60, 63, 69}), "C-Eb-A (Cm6 omit5)");
    
    // Verify normal chords still work correctly
    result.assert_equal("C", get_chord_name({60, 64, 67}), "C-E-G (normal C major)");
    result.assert_equal("Dm", get_chord_name({62, 65, 69}), "D-F-A (normal D minor)");
    result.assert_equal("Csus4", get_chord_name({60, 65, 67}), "C-F-G (C sus4)");
    result.assert_equal("Csus2", get_chord_name({60, 62, 67}), "C-D-G (C sus2)");
    
    // Test incomplete chords
    result.assert_equal("C", get_chord_name({60, 64}), "C-E (major third only)");
    result.assert_equal("Csus4(no5)", get_chord_name({60, 65}), "C-F (perfect fourth only)");
    result.assert_equal("Dm", get_chord_name({62, 65}), "D-F (minor third only)");
}

int main() {
    std::cout << "🎵 Unified Chord Detector - Comprehensive Test Suite 🎵" << std::endl;
    std::cout << std::string(65, '=') << std::endl;

    // Run all test suites
    test_basic_chord_detection();
    test_slash_chord_detection();
    test_analyze_chord_function();
    test_legacy_compatibility();
    test_inversion_analysis();
    test_detailed_analysis();
    test_sharp_flat_notation();
    test_different_input_formats();
    test_edge_cases();
    test_musical_equivalents();
    test_common_progressions();
    test_omit5_and_add11_patterns();
    test_performance();

    // Print final results
    result.print_summary();

    return result.failed > 0 ? 1 : 0;
}
