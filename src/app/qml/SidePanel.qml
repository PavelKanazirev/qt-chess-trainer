import QtQuick
import QtQuick.Controls

Column {
    id: root
    required property var trainer

    spacing: 12

    Label {
        text: "Puzzle " + root.trainer.puzzleId
        font.pixelSize: 22
        font.bold: true
        color: "white"
    }
    Label {
        text: "Rating " + root.trainer.puzzleRating
        color: "#cccccc"
    }
    Label {
        width: root.width
        wrapMode: Text.WordWrap
        text: "Themes: " + root.trainer.themes
        color: "#cccccc"
    }
    Label {
        width: root.width
        wrapMode: Text.WordWrap
        text: root.trainer.statusText
        font.pixelSize: 16
        color: "white"
    }

    Rectangle { width: root.width; height: 1; color: "#555555" }

    Label {
        text: "Your rating: " + root.trainer.playerRating
        color: "white"
    }
    Label {
        text: "Solved " + root.trainer.solvedCount + " / failed " + root.trainer.failedCount
        color: "#cccccc"
    }
    Label {
        text: "Strategy: " + root.trainer.selector
        color: "#cccccc"
    }

    Button {
        text: "Next puzzle"
        onClicked: root.trainer.nextPuzzle()
    }
}
