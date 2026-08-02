import QtQuick
import QtQuick.Controls
import OpenCaddie

Button {
    id: control
    property url iconSource
    property color iconColor: Theme.text
    property string accessibleName
    property bool circular: true
    property bool transparent: false

    width: Theme.touch
    height: Theme.touch
    padding: 12
    display: AbstractButton.IconOnly
    icon.source: iconSource
    icon.width: 24
    icon.height: 24
    icon.color: iconColor
    Accessible.role: Accessible.Button
    Accessible.name: accessibleName

    background: Rectangle {
        radius: control.circular ? width / 2 : Theme.radius
        color: control.transparent ? "transparent"
              : control.down ? Theme.controlPressed : Theme.surface
        border.width: control.transparent ? 0 : 1
        border.color: control.activeFocus ? Theme.fairway : Theme.border
        opacity: control.enabled ? 1 : 0.42
        Behavior on color {
            ColorAnimation { duration: Theme.motionFast }
        }
    }
}
