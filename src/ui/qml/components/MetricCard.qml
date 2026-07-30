import QtQuick
import OpenCaddie

Rectangle {
    id: card
    property string label
    property string value
    property color accent: Theme.text
    property bool prominent: false

    radius: Theme.radius
    color: prominent ? Qt.rgba(0.18, 0.80, 0.39, 0.12) : Theme.surface
    border.width: 1
    border.color: prominent ? Theme.fairway : Theme.border

    Column {
        anchors.centerIn: parent
        spacing: 1
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: card.label
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(11)
            font.weight: Font.Medium
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: card.value
            color: card.accent
            font.family: "Inter"
            font.pixelSize: Theme.px(card.prominent ? 30 : 24)
            font.weight: Font.Bold
        }
    }
}

