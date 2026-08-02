import QtQuick
import OpenCaddie

Rectangle {
    id: card
    property string label
    property string value
    property color accent: Theme.text
    property bool prominent: false

    radius: 0
    color: "transparent"
    border.width: 0

    Column {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 0
        Text {
            width: parent.width
            height: 18
            text: card.label
            color: card.prominent ? Theme.fairway : Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(11)
            font.weight: Font.Medium
            fontSizeMode: Text.Fit
            minimumPixelSize: 8
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        Text {
            width: parent.width
            height: parent.height - 18
            text: card.value
            color: card.accent
            font.family: "Inter"
            font.pixelSize: Theme.px(card.prominent ? 30 : 24)
            font.weight: Font.Bold
            fontSizeMode: Text.Fit
            minimumPixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
