add_library(trainer_api STATIC
    ApiJson.cpp ApiJson.h
    ClientApiServer.cpp ClientApiServer.h)
target_include_directories(trainer_api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(trainer_api
    PUBLIC  trainer_core Qt6::Core
    PRIVATE httplib_header)