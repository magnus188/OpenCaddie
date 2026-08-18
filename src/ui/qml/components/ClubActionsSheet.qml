import QtQuick
import QtQuick.Controls
import OpenCaddie

Popup {
    id: sheet

    signal reorderRequested()
    signal removeRequested()

    parent: Overlay.overlay
    x: 0
    y: parent ? parent.height - height : 0
    width: parent ? parent.width : 800
    height: 190
    modal: true
    focus: true
    padding: 16
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

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
                property: "y"
                from: sheet.parent ? sheet.parent.height : 480
                to: sheet.parent ? sheet.parent.height - sheet.height : 0
                duration: Theme.motionSlow
                easing.type: Easing.OutCubic
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
                property: "y"
                from: sheet.parent ? sheet.parent.height - sheet.height : 0
                to: sheet.parent ? sheet.parent.height : 480
                duration: Theme.motionSheet
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
        spacing: 8

        Text {
            width: parent.width
            height: 30
            text: qsTr("Club actions")
            color: Theme.text
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(18)
            verticalAlignment: Text.AlignVCenter
        }

        AppButton {
            width: parent.width
            height: 56
            text: qsTr("Reorder bag")
            iconSource: "../../assets/icons/lucide/menu.svg"
            variant: "surface"
            onClicked: {
                sheet.close()
                sheet.reorderRequested()
            }
        }

        AppButton {
            width: parent.width
            height: 56
            text: qsTr("Remove club")
            iconSource: "../../assets/icons/lucide/trash-2.svg"
            variant: "danger"
            onClicked: {
                sheet.close()
                sheet.removeRequested()
            }
        }
    }
}
