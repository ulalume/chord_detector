# Chord Detector

High-performance C++17 header-only library for detecting chord names from MIDI note numbers.

## Features

- **Fast**: Optimized for real-time usage (~0.1μs per call)
- **Header-only**: Single file `chord_detector.h`
- **Comprehensive**: Supports triads, 7ths, 9ths, 11ths, slash chords
- **Flexible**: Multiple input formats (arrays, vectors, initializer lists)

## Quick Start

```cpp
#include "chord_detector.h"

// Basic chord detection
std::string chord = get_chord_name({60, 64, 67}); // "C"

// Slash chord detection
std::string slash = get_chord_name({64, 67, 72}, false, true); // "C/E"

// Full analysis
ChordResult result = analyze_chord({64, 67, 72}, false, true);
// result.full_name = "C/E", result.is_slash_chord = true
```

## CMake Integration

```cmake
FetchContent_Declare(
  chord_detector
  GIT_REPOSITORY https://github.com/ulalume/chord_detector
  GIT_TAG        v0.0.1
)
FetchContent_MakeAvailable(chord_detector)

target_link_libraries(your_target chord_detector::chord_detector)
```

## API

### Basic Functions
- `get_chord_name(notes, use_flats=false, use_slash=false)` → string
- `analyze_chord(notes, use_flats=false, use_slash=false)` → ChordResult

### Parameters
- `notes`: MIDI note numbers (C4=60)
- `use_flats`: Use flat notation (Db vs C#)
- `use_slash`: Enable slash chord notation (C/E)

### Supported Chords
- Triads: C, Cm, C+, Co, Csus2, Csus4
- 7ths: C7, Cmaj7, Cm7, CmM7, C7b5, Cm7b5, Co7
- Extended: C6, Cm6, Cadd9, C9, Cmaj9, C11, Cmaj11

## License

MIT
