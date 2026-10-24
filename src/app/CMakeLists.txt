# UI logic without QML, so unit tests can link it
add_library(trainer_ui STATIC
    BoardModel.cpp BoardModel.h
    TrainerBridge.cpp TrainerBridge.h)
target_include_directories(trainer_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(trainer_ui PUBLIC trainer_core Qt6::Core)

qt_add_executable(trainer_app main.cpp)
qt_add_qml_module(trainer_app
    URI ChessTrainer
    VERSION 1.0
    QML_FILES qml/Main.qml qml/Board.qml qml/Square.qml qml/SidePanel.qml)
target_link_libraries(trainer_app PRIVATE trainer_ui Qt6::Quick Qt6::Gui)
target_compile_definitions(trainer_app PRIVATE TRAINER_DATA_DIR="${PROJECT_SOURCE_DIR}/data")
