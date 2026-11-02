import QtQuick

Rectangle {
    id: root

    required property int index
    required property string squareName
    required property string glyph
    required property bool whitePiece
    required property bool light
    required property bool highlighted
    required property bool selected

    signal clicked(int index)

    objectName: "square_" + index
    color: selected ? "#7fa650"
         : highlighted ? (light ? "#f6f669" : "#baca2b")
         : (light ? "#f0d9b5" : "#b58863")

    Text {
        anchors.centerIn: parent
        text: root.glyph
        font.pixelSize: root.height * 0.78
        color: root.whitePiece ? "white" : "black"
        style: Text.Outline
        styleColor: root.whitePiece ? "black" : "#555555"
    }

    Text {   // rank number on the left column
        visible: root.index % 8 === 0
        text: root.squareName.charAt(1)
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 2
        font.pixelSize: root.height * 0.16
        color: root.light ? "#b58863" : "#f0d9b5"
    }

    Text {   // file letter on the bottom row
        visible: root.index >= 56
        text: root.squareName.charAt(0)
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 2
        font.pixelSize: root.height * 0.16
        color: root.light ? "#b58863" : "#f0d9b5"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked(root.index)
    }
}
