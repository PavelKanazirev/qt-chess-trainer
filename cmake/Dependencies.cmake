include(FetchContent)

# chess-library: header-only (MIT). Used only behind ChessPosition.
FetchContent_Declare(chess
  GIT_REPOSITORY https://github.com/Disservin/chess-library.git
  GIT_TAG        53e6a841dcda7059a2af363d85f785ef1817304a
  SOURCE_SUBDIR  no-cmake-here)   # we only need the header, skip the repo's own CMake
FetchContent_MakeAvailable(chess)

add_library(chess_header INTERFACE)
target_include_directories(chess_header SYSTEM INTERFACE ${chess_SOURCE_DIR}/include)

# cpp-httplib: header-only (MIT). A release tag, so a shallow clone is fine.
FetchContent_Declare(httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        v0.66.0
  GIT_SHALLOW    TRUE
  SOURCE_SUBDIR  no-cmake-here)   # we only need httplib.h, skip the repo's own CMake
FetchContent_MakeAvailable(httplib)

find_package(Threads REQUIRED)
add_library(httplib_header INTERFACE)
target_include_directories(httplib_header SYSTEM INTERFACE ${httplib_SOURCE_DIR})
target_link_libraries(httplib_header INTERFACE Threads::Threads)
