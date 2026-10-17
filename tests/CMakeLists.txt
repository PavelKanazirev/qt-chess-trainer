function(add_trainer_test name)
  add_executable(${name} unit/${name}.cpp)
  target_link_libraries(${name} PRIVATE trainer_core Qt6::Test)
  target_compile_definitions(${name} PRIVATE
      TRAINER_DATA_DIR="${PROJECT_SOURCE_DIR}/data")
  add_test(NAME ${name} COMMAND ${name})
endfunction()

add_trainer_test(tst_version)
add_trainer_test(tst_puzzleparser)
add_trainer_test(tst_puzzlerepository)
add_trainer_test(tst_chessposition)

add_test(NAME smoke
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/smoke/smoke.sh $<TARGET_FILE:trainer_app>)
set_tests_properties(smoke PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software"
  TIMEOUT 30)
