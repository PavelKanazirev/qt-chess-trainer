# Qt Chess Trainer

A Qt 6 / QML chess tactics trainer written in C++20. It is a hands-on learning project
with a REST API (cpp-httplib), a separate localhost admin API and a PySide6 admin GUI.

## Set Up
    sudo apt-get update
    sudo apt-get install -y ninja-build qt6-base-dev qt6-declarative-dev qml6-module-qttest fonts-dejavu-core \
        libgl1-mesa-dev \
        qml6-module-qtqml qml6-module-qtqml-workerscript qml6-module-qtqml-models \
        qml6-module-qtquick qml6-module-qtquick-window \
        qml6-module-qtquick-templates qml6-module-qtquick-controls \
        qml6-module-qtquick-layouts

## Build
    The first CMake configure needs network access for FetchContent

    cmake -S . -B build -G Ninja
    cmake --build build
    ctest --test-dir build --output-on-failure

## Documentation

- docs/ProblemDefinition.md : what we build
- docs/Architecture.md : diagrams
- docs/PuzzleFormat.md : puzzle CSV format
