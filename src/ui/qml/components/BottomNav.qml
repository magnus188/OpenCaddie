import QtQuick
import QtQuick.Controls
import OpenCaddie

// Hole navigation uses destination numbers at the edges; the centered dots
// belong only to the information carousel.
Item {
    id: nav

    property int pageCount: 1
    property int currentPage: 0
    property bool lastHole: app.currentHole >= app.holeCount

    signal previous
    signal next
    signal pageSelected(int page)

    implicitHeight: Theme.navigationHeight

    HoleNavButton {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        targetHole: app.currentHole - 1
        reverse: true
        visible: app.currentHole > 1
        accessibleName: qsTr("Previous hole")
        onClicked: nav.previous()
    }

    Row {
        anchors.centerIn: parent
        spacing: -6

        Repeater {
            model: nav.pageCount

            Button {
                required property int index

                width: 42
                height: Theme.touch
                padding: 0
                scale: down ? 0.9 : 1
                Accessible.role: Accessible.Button
                Accessible.name: qsTr("Page %1").arg(index + 1)
                onClicked: nav.pageSelected(index)

                Behavior on scale {
                    NumberAnimation {
                        duration: down ? Theme.motionPress : Theme.motion
                        easing.type: down ? Easing.OutCubic : Easing.OutBack
                        easing.overshoot: 0.6
                    }
                }

                contentItem: Item {
                    Rectangle {
                        anchors.centerIn: parent
                        width: 10
                        height: 10
                        radius: 5
                        color: index === nav.currentPage ? Theme.fairway : Theme.surfaceRaised
                        border.width: index === nav.currentPage ? 0 : 1
                        border.color: Theme.textMuted
                        scale: index === nav.currentPage ? 1.16 : 1

                        Behavior on color {
                            ColorAnimation {
                                duration: Theme.motionFast
                            }
                        }
                        Behavior on scale {
                            NumberAnimation {
                                duration: Theme.motion
                                easing.type: Easing.OutBack
                                easing.overshoot: 0.55
                            }
                        }
                    }
                }

                background: null
            }
        }
    }

    HoleNavButton {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        targetHole: app.currentHole + 1
        completed: nav.lastHole
        accessibleName: nav.lastHole ? qsTr("Finish round") : qsTr("Next hole")
        onClicked: nav.next()
    }

    component HoleNavButton: Button {
        id: control

        property int targetHole: 1
        property bool reverse: false
        property bool completed: false
        property string accessibleName

        width: 94
        height: Theme.touch
        padding: 0
        opacity: enabled ? 1 : 0.32
        scale: control.down ? 0.95 : 1
        Accessible.role: Accessible.Button
        Accessible.name: accessibleName

        Behavior on scale {
            NumberAnimation {
                duration: control.down ? Theme.motionPress : Theme.motion
                easing.type: control.down ? Easing.OutCubic : Easing.OutBack
                easing.overshoot: 0.6
            }
        }
        Behavior on opacity {
            NumberAnimation { duration: Theme.motionFast }
        }

        contentItem: Row {
            anchors.centerIn: parent
            spacing: 8

            Text {
                anchors.verticalCenter: parent.verticalCenter
                visible: control.reverse && !control.completed
                text: "‹"
                color: Theme.text
                font.family: "Inter"
                font.pixelSize: Theme.px(30)
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                visible: control.enabled && !control.completed
                text: control.targetHole
                color: Theme.text
                font.family: "Inter"
                font.weight: Font.Bold
                font.pixelSize: Theme.px(20)
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                visible: !control.reverse
                text: control.completed ? "✓" : "›"
                color: control.completed ? Theme.fairway : Theme.text
                font.family: "Inter"
                font.pixelSize: Theme.px(control.completed ? 20 : 30)
            }
        }

        background: Rectangle {
            radius: height / 2
            color: control.down ? Theme.controlPressed : Theme.surface
            border.width: 1
            border.color: control.completed ? Theme.fairway : Theme.border

            Behavior on color {
                ColorAnimation {
                    duration: Theme.motionFast
                }
            }
        }
    }
}
