import QtQuick
import QtQuick.Controls
import OpenCaddie

AbstractButton {
    id: control
    property string accessibleName: ""

    implicitWidth: 56
    implicitHeight: Theme.touch
    width: implicitWidth
    height: implicitHeight
    checkable: true
    scale: control.down ? 0.96 : 1
    Accessible.role: Accessible.CheckBox
    Accessible.name: accessibleName
    Accessible.checked: checked

    Behavior on scale {
        NumberAnimation {
            duration: control.down ? Theme.motionPress : Theme.motion
            easing.type: control.down ? Easing.OutCubic : Easing.OutBack
            easing.overshoot: 0.55
        }
    }

    background: Item {
        Rectangle {
            id: track
            anchors.centerIn: parent
            width: 48
            height: 28
            radius: height / 2
            color: control.checked ? Theme.greenDeep : Theme.surfaceRaised
            border.width: 1
            border.color: control.checked ? Theme.fairway : Theme.border

            Rectangle {
                x: control.checked ? parent.width - width - 4 : 4
                anchors.verticalCenter: parent.verticalCenter
                width: 20
                height: 20
                radius: width / 2
                color: control.checked ? "#F7F8F2" : Theme.textMuted

                Behavior on x {
                    NumberAnimation {
                        duration: Theme.motionFast
                        easing.type: Easing.OutCubic
                    }
                }
                Behavior on color {
                    ColorAnimation { duration: Theme.motionFast }
                }
            }

            Behavior on color {
                ColorAnimation { duration: Theme.motionFast }
            }
            Behavior on border.color {
                ColorAnimation { duration: Theme.motionFast }
            }
        }
    }
}
