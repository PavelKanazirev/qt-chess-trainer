function(add_trainer_test name)
  add_executable(${name} unit/${name}.cpp)
  target_link_libraries(${name} PRIVATE trainer_core Qt6::Test)
  target_compile_definitions(${name} PRIVATE
      TRAINER_DATA_DIR="${PROJECT_SOURCE_DIR}/data"
      TRAINER_CONFIG_DIR="${PROJECT_SOURCE_DIR}/config")
  add_test(NAME ${name} COMMAND ${name})
endfunction()

find_package(Threads REQUIRED)

add_trainer_test(tst_version)
add_trainer_test(tst_puzzleparser)
add_trainer_test(tst_puzzlerepository)
add_trainer_test(tst_chessposition)
add_trainer_test(tst_movevalidator)
add_trainer_test(tst_puzzlesession)
add_trainer_test(tst_lichessdata)
add_trainer_test(tst_selectors)
add_trainer_test(tst_selectorregistry)
add_trainer_test(tst_statsstore)
add_trainer_test(tst_trainerservice)
add_trainer_test(tst_trainerconcurrency)
add_trainer_test(tst_trainerflow)
add_trainer_test(tst_apijson)
add_trainer_test(tst_clientapi)
add_trainer_test(tst_configservice)
add_trainer_test(tst_adminapi)

target_link_libraries(tst_apijson PRIVATE trainer_api)
target_link_libraries(tst_clientapi PRIVATE trainer_api httplib_header)
target_link_libraries(tst_adminapi PRIVATE trainer_api httplib_header)
target_link_libraries(tst_trainerconcurrency PRIVATE Threads::Threads)

add_test(NAME smoke
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/smoke/smoke.sh $<TARGET_FILE:trainer_app>)
set_tests_properties(smoke PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software"
  TIMEOUT 30)

add_executable(tst_qml qml/tst_qml.cpp)
target_link_libraries(tst_qml PRIVATE Qt6::QuickTest Qt6::Quick)
target_compile_definitions(tst_qml PRIVATE
    QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml")
add_test(NAME tst_qml COMMAND tst_qml)
set_tests_properties(tst_qml PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")

add_trainer_test(tst_boardmodel)
target_link_libraries(tst_boardmodel PRIVATE trainer_ui)
add_trainer_test(tst_trainerbridge)
target_link_libraries(tst_trainerbridge PRIVATE trainer_ui)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  add_test(NAME admin_e2e
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/admin_gui/e2e_check.py
            $<TARGET_FILE:trainer_app>)
  set_tests_properties(admin_e2e PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software"
    TIMEOUT 60)
endif()