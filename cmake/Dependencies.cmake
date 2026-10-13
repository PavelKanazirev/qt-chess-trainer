option(TRAINER_FETCH_DEPS "Fetch cpp-httplib and chess-library" OFF)
if(TRAINER_FETCH_DEPS)
  include(FetchContent)
  FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        <pin a release tag on Day 8>)
  FetchContent_Declare(chess
    GIT_REPOSITORY https://github.com/Disservin/chess-library.git
    GIT_TAG        <pin a release tag on Day 3>)
  FetchContent_MakeAvailable(httplib chess)
endif()
