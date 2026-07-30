import QtQuick
import QtQuick.Controls
import OpenCaddie

ApplicationWindow {
    id: window
    width: 800
    height: 480
    minimumWidth: 800
    minimumHeight: 480
    visible: true
    color: Theme.background
    title: "OpenCaddie"

    Loader {
        id: screenLoader
        anchors.fill: parent
        source: "screens/" + app.screen + ".qml"
        focus: true
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16
        width: Math.min(messageText.implicitWidth + 36, parent.width - 48)
        height: 46
        radius: 23
        color: Theme.overlay
        border.width: 1
        border.color: Theme.border
        visible: app.message.length > 0
        z: 100

        Text {
            id: messageText
            anchors.centerIn: parent
            width: parent.width - 28
            text: app.message
            color: Theme.text
            font.family: "Inter"
            font.pixelSize: Theme.px(14)
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }
}
