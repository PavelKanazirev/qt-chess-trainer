qt_add_executable(trainer_app main.cpp)
qt_add_qml_module(trainer_app
    URI ChessTrainer
    VERSION 1.0
    QML_FILES qml/Main.qml)
target_link_libraries(trainer_app PRIVATE trainer_core Qt6::Quick Qt6::Gui)
