# Qt Chess Trainer

A Qt 6 / QML chess tactics trainer written in C++20. It is a hands-on learning project
with a REST API (cpp-httplib), a separate localhost admin API and a PySide6 admin GUI.


Status: Day 1 of 11, skeleton and CI.

## Build

    cmake -S . -B build -G Ninja
    cmake --build build
    ctest --test-dir build --output-on-failure

## Documentation

- docs/ProblemDefinition.md : what we build
- docs/SolutionPlan.md : how and in which order
- docs/Architecture.md : diagrams
