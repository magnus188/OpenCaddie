import QtQuick
import QtQuick.Controls
import OpenCaddie

Button {
    id: control
    property string variant: "secondary"
    property bool compact: false

    implicitHeight: compact ? Theme.touch : 54
    implicitWidth: Math.max(compact ? 80 : 112, label.implicitWidth + 28)
    padding: compact ? 10 : 14
    font.family: "Inter"
    font.pixelSize: Theme.px(compact ? 14 : 18)
    font.weight: Font.DemiBold

    contentItem: Text {
        id: label
        text: control.text
        color: control.variant === "primary" ? "#F7F8F2"
              : control.variant === "danger" ? "#FFFFFF"
              : Theme.text
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: Theme.radius
        color: control.down
               ? Qt.darker(baseColor, 1.15)
               : control.hovered ? Qt.lighter(baseColor, 1.08) : baseColor
        border.width: control.variant === "primary" ? 2
                      : control.variant === "ghost" ? 0 : 1
        border.color: control.variant === "primary" ? Theme.fairway
                      : control.variant === "danger" ? Theme.danger
                      : Theme.border
        property color baseColor: control.variant === "primary" ? Theme.greenDeep
                                  : control.variant === "danger" ? Theme.danger
                                  : control.variant === "ghost" ? "transparent"
                                  : app.darkMode ? "#0D100E" : Theme.surfaceRaised
        opacity: control.enabled ? 1 : 0.45
    }
}
