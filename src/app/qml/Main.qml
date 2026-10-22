import QtQuick
import QtQuick.Window

Window {
    id: window
    required property var trainer

    width: 900
    height: 620
    visible: true
    title: "Qt Chess Trainer"

    Text {
        anchors.centerIn: parent
        text: "Puzzle " + window.trainer.puzzleId + " (" + window.trainer.puzzleRating + ")"
    }
}

