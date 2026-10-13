add_library(trainer_core STATIC Version.cpp Version.h)
target_include_directories(trainer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(trainer_core PUBLIC Qt6::Core)
target_compile_definitions(trainer_core PRIVATE TRAINER_VERSION="${PROJECT_VERSION}")

