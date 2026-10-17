include(FetchContent)

# chess-library: header-only (MIT). Used only behind ChessPosition.
FetchContent_Declare(chess
  GIT_REPOSITORY https://github.com/Disservin/chess-library.git
  GIT_TAG        53e6a841dcda7059a2af363d85f785ef1817304a
  GIT_SHALLOW    TRUE
  SOURCE_SUBDIR  no-cmake-here)   # we only need the header, skip the repo's own CMake
FetchContent_MakeAvailable(chess)

add_library(chess_header INTERFACE)
target_include_directories(chess_header SYSTEM INTERFACE ${chess_SOURCE_DIR}/include)

# cpp-httplib is fetched on Day 8
option(TRAINER_FETCH_DEPS "Fetch cpp-httplib" OFF)
if(TRAINER_FETCH_DEPS)
  FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        puzzle-data ) # <pin a release tag at the end>
  FetchContent_MakeAvailable(httplib)
endif()
