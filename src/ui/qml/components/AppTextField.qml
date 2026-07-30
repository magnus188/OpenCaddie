import QtQuick
import QtQuick.Controls
import OpenCaddie

TextField {
    id: field
    implicitHeight: Theme.touch
    leftPadding: 14
    rightPadding: 14
    color: Theme.text
    placeholderTextColor: Theme.textMuted
    selectionColor: Theme.greenDeep
    selectedTextColor: "#FFFFFF"
    font.family: "Inter"
    font.pixelSize: Theme.px(16)
    background: Rectangle {
        radius: Theme.radius
        color: Theme.surfaceRaised
        border.width: field.activeFocus ? 2 : 1
        border.color: field.activeFocus ? Theme.fairway : Theme.border
    }
}

