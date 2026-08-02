import QtQuick
import QtQuick.Controls
import OpenCaddie

Button {
    id: control
    property string variant: "secondary"
    property bool compact: false

    implicitHeight: compact ? Theme.touch : 52
    implicitWidth: Math.max(compact ? 64 : 104, label.implicitWidth + 28)
    padding: compact ? 8 : 12
    font.family: "Inter"
    font.pixelSize: Theme.px(compact ? 14 : Theme.control)
    font.weight: Font.DemiBold
    Accessible.role: Accessible.Button
    Accessible.name: text

    contentItem: Text {
        id: label
        text: control.text
        color: control.variant === "primary" ? "#F7F8F2"
              : control.variant === "danger" ? Theme.danger
              : control.variant === "accent" ? Theme.fairway
              : Theme.text
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
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
    }
}
