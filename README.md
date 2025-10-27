# Chord Detector

High-performance C++17 header-only library for detecting chord names from MIDI note numbers.

## Features

- **Fast**: Optimized for real-time usage (~0.25μs per call)
- **Header-only**: Single file `chord_detector.h` - no dependencies
- **Enhanced**: omit5 patterns, add11 chords, complex slash chord analysis
- **Comprehensive**: 50+ chord types including triads, 7ths, 9ths, 11ths, extensions
- **Flexible**: Multiple input formats (arrays, vectors, initializer lists)

## Quick Start

```cpp
#include "chord_detector.h"

// Basic chord detection
std::string chord = get_chord_name({60, 64, 67}); // "C"

// Enhanced omit5 detection (NEW)
std::string omit5 = get_chord_name({60, 64, 65}); // "Cadd11(omit5)"

// Slash chord detection
std::string slash = get_chord_name({60, 62, 65}, false, true); // "Dm7(omit5)/C"

// Full analysis
ChordResult result = analyze_chord({64, 67, 72}, false, true);
// result.full_name = "C/E", result.is_slash_chord = true

// Detailed analysis with inversions
DetailedAnalysis detailed = get_detailed_analysis({67, 71, 74, 77});
// detailed.inversion_type = "1st", detailed.note_names = {"G", "B", "D", "F"}
```

## CMake Integration

```cmake
include(FetchContent)

FetchContent_Declare(
  chord_detector
  GIT_REPOSITORY https://github.com/ulalume/chord_detector
  GIT_TAG        HEAD
)
FetchContent_MakeAvailable(chord_detector)

target_link_libraries(your_target chord_detector::chord_detector)
```

## API

### Core Functions
- `get_chord_name(notes, use_flats=false, use_slash=false)` → string
- `analyze_chord(notes, use_flats=false, use_slash=false)` → ChordResult
- `get_detailed_analysis(notes, use_flats=false)` → DetailedAnalysis

### Parameters
- `notes`: MIDI note numbers (C4=60)
- `use_flats`: Use flat notation (Db vs C#)
- `use_slash`: Enable slash chord notation (C/E)

### Supported Chords
- **Triads**: C, Cm, C+, Co, Csus2, Csus4
- **7ths**: C7, Cmaj7, Cm7, CmM7, C7b5, Cm7b5, Co7
- **Extended**: C6, Cm6, Cadd9, C9, Cmaj9, C11, Cmaj11
- **omit5 (NEW)**: C7(omit5), Cmaj7(omit5), Cadd11(omit5)
- **Slash/Inversions**: C/E, Am/C, G7/B, Dm7(omit5)/C

## License

MIT
