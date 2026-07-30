import QtQuick
import OpenCaddie

Rectangle {
    id: keyboard
    property var target
    signal done()

    height: 204
    color: Theme.surface
    border.width: 1
    border.color: Theme.border
    radius: Theme.radius

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
        anchors.margins: 8
        spacing: 5

        Repeater {
            model: ["QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"]
            Row {
                required property string modelData
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 5
                Repeater {
                    model: modelData.length
                    AppButton {
                        required property int index
                        width: 46
                        height: 44
                        compact: true
                        text: modelData.charAt(index)
                        onClicked: keyboard.insert(text.toLowerCase())
                    }
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 6
            AppButton {
                width: 112
                compact: true
                text: qsTr("Delete")
                onClicked: keyboard.erase()
            }
            AppButton {
                width: 270
                compact: true
                text: qsTr("Space")
                onClicked: keyboard.insert(" ")
            }
            AppButton {
                width: 112
                compact: true
                variant: "primary"
                text: qsTr("Done")
                onClicked: keyboard.done()
            }
        }
    }
}

