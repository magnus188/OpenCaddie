import QtQuick
import QtQuick.Controls
import OpenCaddie

Dialog {
    id: dialog
    property string bodyText
    property bool destructive: false
    property string confirmText: qsTr("Confirm")
    signal confirmed()

    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 64 : 520, 520)
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: 0
    header: null
    footer: null

    enter: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: Theme.motionSheet
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                property: "scale"
                from: 0.975
                to: 1
                duration: Theme.motionSlow
                easing.type: Easing.OutBack
                easing.overshoot: 0.55
            }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: Theme.motionFast
                easing.type: Easing.InCubic
            }
            NumberAnimation {
                property: "scale"
                from: 1
                to: 0.985
                duration: Theme.motionFast
                easing.type: Easing.InCubic
            }
        }
    }

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
                variant: "surface"
                onClicked: dialog.close()
            }
            AppButton {
                text: dialog.confirmText
                variant: dialog.destructive ? "danger" : "primary"
                onClicked: {
                    dialog.confirmed()
                    dialog.close()
                }
            }
        }
    }
}
