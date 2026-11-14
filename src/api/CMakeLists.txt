add_library(trainer_api STATIC
    ApiJson.cpp ApiJson.h
    ApiServerBase.cpp ApiServerBase.h
    ClientApiServer.cpp ClientApiServer.h
    AdminApiServer.cpp AdminApiServer.h)
target_include_directories(trainer_api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(trainer_api
    PUBLIC  trainer_core Qt6::Core
    PRIVATE httplib_header)