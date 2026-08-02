import QtQuick
import QtQuick.Controls
import OpenCaddie

Dialog {
    id: dialog
    property string bodyText
    signal confirmed()

    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 64 : 520, 520)
    modal: true
    closePolicy: Popup.CloseOnEscape
    padding: 0

    background: Rectangle {
        color: Theme.surface
        radius: Theme.sheetRadius
        border.width: 1
        border.color: Theme.border
    }

    contentItem: Column {
        padding: 20
        spacing: 16
        Text {
            width: parent.width - 40
            text: dialog.title
            color: Theme.text
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(22)
            wrapMode: Text.WordWrap
        }
        Text {
            width: parent.width - 40
            text: dialog.bodyText
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(15)
            wrapMode: Text.WordWrap
        }
        Row {
            anchors.right: parent.right
            spacing: 10
            AppButton {
                text: qsTr("Cancel")
                variant: "accent"
                onClicked: dialog.close()
            }
            AppButton {
                text: qsTr("Confirm")
                variant: "danger"
                onClicked: {
                    dialog.confirmed()
                    dialog.close()
                }
            }
        }
    }
}
