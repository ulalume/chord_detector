cmake_minimum_required(VERSION 3.14)

project(chord_detector
    VERSION 0.0.1
    DESCRIPTION "High-performance chord detector for MIDI notes"
    LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create interface library for header-only usage
add_library(chord_detector INTERFACE)
add_library(chord_detector::chord_detector ALIAS chord_detector)

# Set include directories
target_include_directories(chord_detector INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

# Install header file
install(FILES chord_detector.h
    DESTINATION include)

# Install targets
install(TARGETS chord_detector
    EXPORT chord_detector_targets
    INCLUDES DESTINATION include)

# Install export targets
install(EXPORT chord_detector_targets
    FILE chord_detector-targets.cmake
    NAMESPACE chord_detector::
    DESTINATION lib/cmake/chord_detector)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    chord_detector-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/chord_detector-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/chord_detector-config.cmake
    INSTALL_DESTINATION lib/cmake/chord_detector)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/chord_detector-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/chord_detector-config-version.cmake
    DESTINATION lib/cmake/chord_detector)

# Build test executable if this is the main project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  add_executable(chord_detector_test test.cpp)
  target_link_libraries(chord_detector_test chord_detector::chord_detector)

  enable_testing()
  add_test(NAME chord_detector_test COMMAND chord_detector_test)
endif()
