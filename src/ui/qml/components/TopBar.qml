import QtQuick
import OpenCaddie

Item {
    id: bar
    property string title
    property string subtitle
    property bool showBack: true
    signal back()

    implicitHeight: 78

    AppButton {
        id: backButton
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        text: "‹  " + qsTr("Back")
        variant: "ghost"
        compact: true
        visible: bar.showBack
        onClicked: bar.back()
    }

    Column {
        anchors.left: showBack ? backButton.right : parent.left
        anchors.leftMargin: showBack ? 8 : 0
        anchors.verticalCenter: parent.verticalCenter
        spacing: 1

        Text {
            text: bar.title
            color: Theme.text
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(30)
        }
        Text {
            text: bar.subtitle
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(12)
            visible: text.length > 0
        }
    }
}
