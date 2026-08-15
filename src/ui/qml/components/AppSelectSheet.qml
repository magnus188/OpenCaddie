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
        text: control.displayText + "  ⌄"
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
