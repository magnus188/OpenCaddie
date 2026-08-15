import QtQuick
import QtQuick.Controls
import OpenCaddie

TextField {
    id: field
    // Opens the shared on-screen keyboard in numeric mode when true.
    property bool numericKeyboard: false
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
        color: Theme.surface
        border.width: 1
        border.color: field.activeFocus ? Theme.fairway : Theme.border
        Behavior on border.color {
            ColorAnimation { duration: Theme.motionFast }
        }
    }

    TapHandler {
        enabled: field.enabled
        onTapped: {
            field.forceActiveFocus()
            KeyboardController.open(field, field.numericKeyboard)
        }
    }

    Connections {
        target: KeyboardController
        function onCommitRequested() {
            if (KeyboardController.target === field)
                field.accepted()
        }
    }
}
