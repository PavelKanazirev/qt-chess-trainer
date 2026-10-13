add_executable(tst_version unit/tst_version.cpp)
target_link_libraries(tst_version PRIVATE trainer_core Qt6::Test)
add_test(NAME tst_version COMMAND tst_version)

add_test(NAME smoke
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/smoke/smoke.sh $<TARGET_FILE:trainer_app>)
set_tests_properties(smoke PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software"
  TIMEOUT 30)