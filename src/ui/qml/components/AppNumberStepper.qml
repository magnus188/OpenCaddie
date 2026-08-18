import QtQuick
import OpenCaddie

Item {
    id: control

    property int value: 1
    property int from: 1
    property int to: 350
    property string unit: ""
    property string accessibleName: qsTr("Value")
    readonly property bool editing: valueField.activeFocus
    readonly property bool keyboardActive:
        KeyboardController.target === valueField
    signal valueEdited(int newValue)

    implicitWidth: 248
    implicitHeight: Theme.touch

    function bounded(value) {
        return Math.max(control.from, Math.min(control.to, value))
    }

    function parsedText() {
        const trimmed = valueField.text.trim()
        if (trimmed.length === 0)
            return NaN
        const parsed = Number(trimmed)
        return isFinite(parsed) ? Math.round(parsed) : NaN
    }

    function syncText() {
        valueField.text = String(control.bounded(control.value))
    }

    function commitText() {
        const parsed = control.parsedText()
        if (!isFinite(parsed)) {
            control.syncText()
            return
        }
        const next = control.bounded(parsed)
        valueField.text = String(next)
        if (next !== control.value)
            control.valueEdited(next)
    }

    function step(delta) {
        const parsed = control.parsedText()
        const base = isFinite(parsed) ? control.bounded(parsed)
                                      : control.bounded(control.value)
        const next = control.bounded(base + delta)
        valueField.text = String(next)
        if (next !== control.value)
            control.valueEdited(next)
    }

    onValueChanged: {
        if (!valueField.activeFocus)
            syncText()
    }
    onFromChanged: syncText()
    onToChanged: syncText()
    Component.onCompleted: syncText()

    Row {
        anchors.fill: parent
        spacing: 8

        IconButton {
            width: Theme.touch
            height: Theme.touch
            iconSource: "../../assets/icons/lucide/minus.svg"
            iconColor: Theme.text
            accessibleName: qsTr("Decrease carry")
            enabled: control.value > control.from
            onClicked: control.step(-1)
        }

        Item {
            width: control.width - Theme.touch * 2 - 16
            height: Theme.touch

            AppTextField {
                id: valueField
                anchors.fill: parent
                rightPadding: unitLabel.visible ? 42 : 14
                horizontalAlignment: TextInput.AlignHCenter
                inputMethodHints: Qt.ImhDigitsOnly
                numericKeyboard: true
                font.pixelSize: Theme.px(20)
                font.weight: Font.Bold
                Accessible.name: control.accessibleName
                validator: RegularExpressionValidator {
                    regularExpression: /^[0-9]*$/
                }
                onAccepted: control.commitText()
                onActiveFocusChanged: {
                    if (!activeFocus)
                        control.commitText()
                }
            }

            Text {
                id: unitLabel
                anchors.right: parent.right
                anchors.rightMargin: 13
                anchors.verticalCenter: parent.verticalCenter
                text: control.unit
                color: Theme.textMuted
                font.family: "Inter"
                font.weight: Font.DemiBold
                font.pixelSize: Theme.px(12)
                visible: text.length > 0
            }
        }

        IconButton {
            width: Theme.touch
            height: Theme.touch
            iconSource: "../../assets/icons/lucide/plus.svg"
            iconColor: Theme.text
            accessibleName: qsTr("Increase carry")
            enabled: control.value < control.to
            onClicked: control.step(1)
        }
    }
}
