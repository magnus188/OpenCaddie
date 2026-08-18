import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    id: control
    property var model: []
    property string textRole: "text"
    property string valueRole: "value"
    property int currentIndex: 0
    readonly property var currentValue: valueAt(currentIndex)
    readonly property string displayText: textAt(currentIndex)
    property string buttonText: displayText + "  ⌄"
    property string accessibleName: displayText
    signal activated()

    implicitWidth: 250
    implicitHeight: Theme.touch

    function itemAt(index) {
        return model && index >= 0 && index < model.length ? model[index] : null
    }

    function textAt(index) {
        const item = itemAt(index)
        return item && item[textRole] !== undefined ? String(item[textRole]) : ""
    }

    function valueAt(index) {
        const item = itemAt(index)
        return item && item[valueRole] !== undefined ? item[valueRole] : undefined
    }

    AppButton {
        anchors.fill: parent
        text: control.buttonText
        accessibleName: control.accessibleName
        variant: "surface"
        compact: true
        onClicked: sheet.open()
    }

    Popup {
        id: sheet
        parent: Overlay.overlay
        x: 0
        y: parent ? parent.height - height : 0
        width: parent ? parent.width : 800
        height: Math.min(parent ? parent.height - Theme.statusHeight : 420,
                         Math.max(140, optionList.contentHeight + 40))
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

        contentItem: ListView {
            id: optionList
            model: control.model
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollIndicator.vertical: ScrollIndicator { }

            delegate: Rectangle {
                required property var modelData
                required property int index
                width: optionList.width
                height: 56
                radius: Theme.radius
                color: index === control.currentIndex
                       ? Qt.rgba(0.18, 0.80, 0.39, 0.14)
                       : optionTap.pressed ? Theme.controlPressed : "transparent"

                Behavior on color {
                    ColorAnimation { duration: Theme.motionFast }
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: checkMark.left
                    anchors.leftMargin: 16
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: control.textAt(index)
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.DemiBold
                    font.pixelSize: Theme.px(16)
                    elide: Text.ElideRight
                }

                Text {
                    id: checkMark
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    text: index === control.currentIndex ? "✓" : ""
                    color: Theme.fairway
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(20)
                }

                TapHandler {
                    id: optionTap
                    onTapped: {
                        control.currentIndex = index
                        control.activated()
                        sheet.close()
                    }
                }
            }
        }
    }
}
