import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import OpenCaddie

Button {
    id: control
    property string variant: "secondary"
    property bool compact: false
    property url iconSource
    property string accessibleName: ""
    readonly property color foregroundColor:
        control.variant === "primary" ? "#F7F8F2"
        : control.variant === "danger" ? Theme.danger
        : control.variant === "accent" ? Theme.fairway
        : Theme.text

    implicitHeight: compact ? Theme.touch : 52
    implicitWidth: Math.max(compact ? 64 : 104,
                            buttonContent.implicitWidth + 28)
    padding: compact ? 8 : 12
    font.family: "Inter"
    font.pixelSize: Theme.px(compact ? 14 : Theme.control)
    font.weight: Font.DemiBold
    scale: control.down ? 0.975 : 1
    Accessible.role: Accessible.Button
    Accessible.name: accessibleName.length > 0 ? accessibleName : text

    Behavior on scale {
        NumberAnimation {
            duration: control.down ? Theme.motionPress : Theme.motion
            easing.type: control.down ? Easing.OutCubic : Easing.OutBack
            easing.overshoot: 0.65
        }
    }

    contentItem: Item {
        implicitWidth: buttonContent.implicitWidth
        implicitHeight: Math.max(buttonContent.implicitHeight, 24)

        Row {
            id: buttonContent
            anchors.centerIn: parent
            spacing: control.text.length > 0 && buttonIcon.visible ? 8 : 0

            Item {
                id: buttonIcon
                anchors.verticalCenter: parent.verticalCenter
                width: 20
                height: 20
                visible: control.iconSource.toString().length > 0

                Image {
                    id: buttonIconSource
                    anchors.fill: parent
                    source: control.iconSource
                    sourceSize.width: 20
                    sourceSize.height: 20
                    fillMode: Image.PreserveAspectFit
                    visible: false
                }

                MultiEffect {
                    anchors.fill: parent
                    source: buttonIconSource
                    brightness: 1
                    colorization: 1
                    colorizationColor: control.foregroundColor
                }
            }

            Text {
                id: label
                anchors.verticalCenter: parent.verticalCenter
                text: control.text
                color: control.foregroundColor
                font: control.font
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight

                Behavior on color {
                    ColorAnimation { duration: Theme.motionFast }
                }
            }
        }
    }

    background: Rectangle {
        radius: Theme.radius
        color: control.down ? pressedColor
              : control.hovered && control.variant !== "primary"
                ? Theme.controlPressed : baseColor
        border.width: control.activeFocus ? 1
                      : control.variant === "surface" ? 1 : 0
        border.color: control.activeFocus ? Theme.fairway : Theme.border
        property color baseColor: control.variant === "primary" ? Theme.greenDeep
                                  : control.variant === "surface"
                                    ? Theme.surfaceRaised : "transparent"
        property color pressedColor: control.variant === "primary"
                                     ? Qt.darker(Theme.greenDeep, 1.12)
                                     : Theme.controlPressed
        opacity: control.enabled ? 1 : 0.45

        Behavior on color {
            ColorAnimation { duration: Theme.motionFast }
        }
        Behavior on border.color {
            ColorAnimation { duration: Theme.motionFast }
        }
        Behavior on opacity {
            NumberAnimation { duration: Theme.motionFast }
        }
    }
}
