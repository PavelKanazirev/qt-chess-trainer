import QtQuick

Rectangle {
    id: root

    required property int index
    required property string squareName
    required property string glyph
    required property bool whitePiece
    required property bool light
    required property bool highlighted

    signal clicked(int index)

    color: highlighted ? (light ? "#f6f669" : "#baca2b")
                       : (light ? "#f0d9b5" : "#b58863")

    Text {
        anchors.centerIn: parent
        text: root.glyph
        font.pixelSize: root.height * 0.78
        color: root.whitePiece ? "white" : "black"
        style: Text.Outline
        styleColor: root.whitePiece ? "black" : "#555555"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked(root.index)
    }
}
