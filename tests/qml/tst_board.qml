import QtQuick
import QtTest
import "../../src/app/qml"

Item {
    width: 480
    height: 480

    ListModel {
        id: squares
        Component.onCompleted: {
            for (let i = 0; i < 64; ++i) {
                append({
                    squareName: "s" + i,
                    glyph: i === 0 ? "\u265C" : "",
                    whitePiece: false,
                    light: (Math.floor(i / 8) + i) % 2 === 0,
                    highlighted: i === 63
                })
            }
        }
    }

    Board {
        id: board
        anchors.fill: parent
        model: squares
    }

    SignalSpy {
        id: clickSpy
        target: board
        signalName: "squareClicked"
    }

    TestCase {
        name: "Board"
        when: windowShown

        function test_has64Squares() {
            compare(board.squareCount, 64)
        }

        function test_clickReportsDisplayIndex() {
            clickSpy.clear()
            mouseClick(board, 30, 30)                      // top-left square
            compare(clickSpy.count, 1)
            compare(clickSpy.signalArguments[0][0], 0)

            mouseClick(board, board.width - 30, board.height - 30)   // bottom-right
            compare(clickSpy.count, 2)
            compare(clickSpy.signalArguments[1][0], 63)
        }
    }
}
