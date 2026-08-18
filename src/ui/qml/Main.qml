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
        transform: Translate { id: screenSlide }
        onLoaded: screenEnter.restart()

        // Slide direction follows the back-stack: push arrives from the right,
        // pop from the left, flow resets fade in place. Transform-only, so it
        // stays cheap on the Pi's EGLFS stack.
        ParallelAnimation {
            id: screenEnter
            NumberAnimation {
                target: screenSlide
                property: "x"
                from: app.navigationDirection * 32
                to: 0
                duration: Theme.motion
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: screenLoader
                property: "opacity"
                from: 0
                to: 1
                duration: Theme.motion
                easing.type: Easing.OutCubic
            }
        }
    }

    Connections {
        target: app
        function onScreenChanged() {
            KeyboardController.close()
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

    OnScreenKeyboard {
        id: keyboardHost
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        target: KeyboardController.target
        visible: KeyboardController.active
        z: 95
        onDone: {
            KeyboardController.commitRequested()
            KeyboardController.close()
        }
    }

    Connections {
        target: KeyboardController
        function onTargetChanged() {
            keyboardHost.numeric = KeyboardController.numeric
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Theme.statusHeight + 8
        width: Math.min(messageText.implicitWidth + 36, parent.width - 48)
        height: 46
        radius: 23
        color: Theme.overlay
        border.width: 1
        border.color: Theme.border
        visible: opacity > 0
        opacity: app.message.length > 0 ? 1 : 0
        scale: app.message.length > 0 ? 1 : 0.98
        z: 100

        transform: Translate {
            y: app.message.length > 0 ? 0 : 10
            Behavior on y {
                NumberAnimation {
                    duration: Theme.motionSheet
                    easing.type: Easing.OutCubic
                }
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: Theme.motion }
        }
        Behavior on scale {
            NumberAnimation {
                duration: Theme.motionSheet
                easing.type: Easing.OutBack
                easing.overshoot: 0.5
            }
        }

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

    // Skipped for --screenshot runs so captured screens stay deterministic.
    Loader {
        anchors.fill: parent
        active: !screenshotMode
        sourceComponent: SplashOverlay {}
        z: 200
    }
}
