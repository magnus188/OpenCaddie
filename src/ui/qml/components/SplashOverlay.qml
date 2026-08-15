import QtQuick
import OpenCaddie

// Covers the whole window, status bar included, while the engine finishes waking
// up. The mark arrives first and the word follows it, so the boot reads as the
// arc being drawn and then named. Tap to skip; it never blocks for long.
Rectangle {
    id: splash

    color: Theme.focusBackground
    visible: opacity > 0
    z: 200

    BrandLockup {
        id: lockup
        anchors.centerIn: parent
        markHeight: 72
        markItem.opacity: 0
        wordItem.opacity: 0
    }

    MouseArea {
        anchors.fill: parent
        onClicked: introduction.complete()
    }

    SequentialAnimation {
        id: introduction
        running: true

        ParallelAnimation {
            NumberAnimation {
                target: lockup.markItem
                property: "opacity"
                to: 1
                duration: 360
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: lockup.markItem
                property: "scale"
                from: 0.86
                to: 1
                duration: 420
                easing.type: Easing.OutCubic
            }
            SequentialAnimation {
                PauseAnimation { duration: 170 }
                NumberAnimation {
                    target: lockup.wordItem
                    property: "opacity"
                    to: 1
                    duration: 300
                    easing.type: Easing.OutCubic
                }
            }
        }

        PauseAnimation { duration: 620 }

        NumberAnimation {
            target: splash
            property: "opacity"
            to: 0
            duration: Theme.motionSheet
            easing.type: Easing.InCubic
        }
    }
}
