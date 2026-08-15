import QtQuick
import OpenCaddie

// Mark plus wordmark, laid out to the brand spec in docs/brand: the wordmark's
// cap height is 60% of the mark height, and the gap between them is half of it.
Row {
    id: lockup

    property int markHeight: 72
    property color textColor: Theme.text
    // Exposed so an introduction can stage the mark ahead of the word.
    property alias markItem: mark
    property alias wordItem: word

    // Inter's cap height is 0.727em, and the mark's artwork is 50x48 units.
    readonly property int wordSize: Math.round(markHeight * 0.6 / 0.727)

    spacing: Math.round(markHeight / 2)

    Image {
        id: mark
        source: "../../assets/brand/mark.svg"
        height: lockup.markHeight
        width: Math.round(lockup.markHeight * 50 / 48)
        sourceSize.width: width * 2
        sourceSize.height: height * 2
        fillMode: Image.PreserveAspectFit
        anchors.verticalCenter: parent.verticalCenter
    }

    Row {
        id: word
        spacing: 0
        anchors.verticalCenter: parent.verticalCenter
        // Inter sits slightly low in its box; nudge the word onto the mark's axis.
        anchors.verticalCenterOffset: -Math.round(lockup.wordSize * 0.04)

        Text {
            text: "Open"
            color: lockup.textColor
            font.family: "Inter"
            font.pixelSize: lockup.wordSize
            font.weight: Font.Medium
            font.letterSpacing: -lockup.wordSize * 0.01
        }

        Text {
            text: "Caddie"
            color: lockup.textColor
            font.family: "Inter"
            font.pixelSize: lockup.wordSize
            font.weight: Font.Bold
            font.letterSpacing: -lockup.wordSize * 0.01
        }
    }
}
