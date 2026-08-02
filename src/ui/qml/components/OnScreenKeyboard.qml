import QtQuick
import OpenCaddie

Rectangle {
    id: keyboard
    property var target
    property bool shifted: false
    property bool numeric: false
    signal done()

    height: 208
    color: Theme.surface
    border.width: 1
    border.color: Theme.border
    radius: Theme.radius
    onVisibleChanged: {
        if (!visible) {
            shifted = false
            numeric = false
        }
    }

    function insert(value) {
        if (!target)
            return
        var start = target.selectionStart
        var end = target.selectionEnd
        target.text = target.text.slice(0, start) + value + target.text.slice(end)
        target.cursorPosition = start + value.length
        target.forceActiveFocus()
    }

    function erase() {
        if (!target)
            return
        var start = target.selectionStart
        var end = target.selectionEnd
        if (start === end && start > 0)
            start--
        target.text = target.text.slice(0, start) + target.text.slice(end)
        target.cursorPosition = start
        target.forceActiveFocus()
    }

    Column {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 2

        Repeater {
            model: keyboard.numeric
                   ? ["1234567890"]
                   : ["QWERTYUIOP", "ASDFGHJKL", "ZXCVBNMÆØÅ"]
            Row {
                id: keyRow
                required property string modelData
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 5
                Repeater {
                    model: modelData.length
                    AppButton {
                        required property int index
                        width: 48
                        height: 48
                        compact: true
                        variant: "surface"
                        text: keyRow.modelData.charAt(index)
                        onClicked: keyboard.insert(
                            keyboard.shifted ? text : text.toLowerCase())
                    }
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 5
            AppButton {
                width: 70
                height: 48
                compact: true
                variant: keyboard.numeric ? "primary" : "surface"
                text: keyboard.numeric ? "ABC" : "123"
                onClicked: {
                    keyboard.numeric = !keyboard.numeric
                    keyboard.shifted = false
                }
            }
            AppButton {
                visible: !keyboard.numeric
                width: 70
                height: 48
                compact: true
                variant: keyboard.shifted ? "primary" : "surface"
                text: "⇧"
                onClicked: keyboard.shifted = !keyboard.shifted
            }
            AppButton {
                width: 86
                height: 48
                compact: true
                variant: "surface"
                text: qsTr("Delete")
                onClicked: keyboard.erase()
            }
            Repeater {
                model: [".", "-", "/", ":", "@"]
                AppButton {
                    required property string modelData
                    width: 48
                    height: 48
                    compact: true
                    variant: "surface"
                    text: modelData
                    onClicked: keyboard.insert(text)
                }
            }
            AppButton {
                width: 140
                height: 48
                compact: true
                variant: "surface"
                text: qsTr("Space")
                onClicked: keyboard.insert(" ")
            }
            AppButton {
                width: 86
                height: 48
                compact: true
                variant: "primary"
                text: qsTr("Done")
                onClicked: keyboard.done()
            }
        }
    }
}
