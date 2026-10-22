import QtQuick
import QtQuick.Window

Window {
    id: window
    required property var trainer

    width: 900
    height: 620
    minimumWidth: 640
    minimumHeight: 420
    visible: true
    title: "Qt Chess Trainer"
    color: "#302e2b"

    Board {
        id: board
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 16
        width: Math.min(parent.width - 300, parent.height - 32)
        model: window.trainer.board
    }
}
