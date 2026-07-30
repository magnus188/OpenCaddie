import QtQuick
import OpenCaddie

Rectangle {
    id: card
    property alias title: titleLabel.text
    property alias subtitle: subtitleLabel.text
    property alias contentItem: body.data
    default property alias contents: body.data

    color: "transparent"
    radius: Theme.radius
    border.width: 1
    border.color: Theme.border
    implicitHeight: body.childrenRect.height + (titleLabel.text.length > 0 ? 58 : 24)

    Text {
        id: titleLabel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 14
        color: Theme.text
        font.family: "Inter"
        font.weight: Font.Bold
        font.pixelSize: Theme.px(18)
        elide: Text.ElideRight
    }

    Text {
        id: subtitleLabel
        anchors.left: titleLabel.left
        anchors.right: titleLabel.right
        anchors.top: titleLabel.bottom
        anchors.topMargin: 2
        color: Theme.textMuted
        font.family: "Inter"
        font.pixelSize: Theme.px(12)
        visible: text.length > 0
        elide: Text.ElideRight
    }

    Item {
        id: body
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: titleLabel.text.length > 0 ? subtitleLabel.bottom : parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 14
        anchors.topMargin: titleLabel.text.length > 0 ? 10 : 12
    }
}
