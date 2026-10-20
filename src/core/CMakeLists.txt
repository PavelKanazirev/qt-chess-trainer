add_library(trainer_core STATIC
    Version.cpp Version.h
    Puzzle.h
    PuzzleParser.cpp PuzzleParser.h
    PuzzleRepository.cpp PuzzleRepository.h
    ChessPosition.cpp ChessPosition.h
    MoveValidator.cpp MoveValidator.h
    PuzzleSession.cpp PuzzleSession.h
    selectors/IPuzzleSelector.h
    selectors/RandomSelector.cpp selectors/RandomSelector.h
    selectors/ByRatingSelector.cpp selectors/ByRatingSelector.h
    selectors/WeakestThemeSelector.cpp selectors/WeakestThemeSelector.h
    selectors/SelectorRegistry.cpp selectors/SelectorRegistry.h
    StatsStore.cpp StatsStore.h
    TrainerTypes.h TrainerService.cpp TrainerService.h
    )
target_include_directories(trainer_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/selectors)
target_link_libraries(trainer_core
    PUBLIC  Qt6::Core
    PRIVATE chess_header)
target_compile_definitions(trainer_core PRIVATE TRAINER_VERSION="${PROJECT_VERSION}")
