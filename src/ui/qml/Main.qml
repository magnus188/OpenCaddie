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
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: Theme.statusHeight
        anchors.bottom: parent.bottom
        source: "screens/" + app.screen + ".qml"
        focus: true
        opacity: status === Loader.Ready ? 1 : 0
        scale: (app.screen === "RoundMapScreen" ||
                app.screen === "CoursePlannerMapScreen") && status === Loader.Ready
               ? 1 : 0.992
        Behavior on opacity {
            NumberAnimation { duration: Theme.motion; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            NumberAnimation { duration: Theme.motionSheet; easing.type: Easing.OutCubic }
        }
    }

    Connections {
        target: app
        function onScoreEntryRequested(hole) {
            app.screen = "HoleScoreScreen"
        }
    }

    SystemStatusBar {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        z: 90
    }

    ScoreCelebrationOverlay {
        anchors.fill: parent
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
