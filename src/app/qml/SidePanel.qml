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

    Rectangle {
        width: root.width
        height: statusLabel.implicitHeight + 20
        radius: 6
        color: {
            switch (root.trainer.feedback) {
            case "solved":  return "#2e7d32"
            case "correct": return "#558b2f"
            case "illegal": return "#ef6c00"
            case "wrong":   return "#c62828"
            default:        return "#444444"
            }
        }
        Behavior on color { ColorAnimation { duration: 150 } }

        Label {
            id: statusLabel
            anchors.fill: parent
            anchors.margins: 10
            wrapMode: Text.WordWrap
            text: root.trainer.statusText
            font.pixelSize: 16
            color: "white"
        }
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

    Label { text: "Strategy"; color: "#cccccc" }
    ComboBox {
        width: root.width
        model: root.trainer.availableSelectors
        currentIndex: root.trainer.availableSelectors.indexOf(root.trainer.selector)
        onActivated: (i) => root.trainer.setSelector(textAt(i))
    }

    Button {
        text: root.trainer.finished ? "Next puzzle (N)" : "Skip puzzle (N)"
        onClicked: root.trainer.nextPuzzle()
    }
}
