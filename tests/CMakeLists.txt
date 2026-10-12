add_executable(tst_version unit/tst_version.cpp)
target_link_libraries(tst_version PRIVATE trainer_core Qt6::Test)
add_test(NAME tst_version COMMAND tst_version)
