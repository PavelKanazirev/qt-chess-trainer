import QtQuick

// 8x8 grid. Works with any model providing the roles Square.qml requires.
Item {
    id: root

    property alias model: repeater.model
    readonly property int squareCount: repeater.count

    signal squareClicked(int index)

    Grid {
        id: grid
        readonly property real side: Math.min(root.width, root.height)
        anchors.centerIn: parent
        width: side
        height: side
        columns: 8

        Repeater {
            id: repeater
            Square {
                width: grid.side / 8
                height: grid.side / 8
                onClicked: (i) => root.squareClicked(i)
            }
        }
    }
}
