import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    id: control
    property alias from: slider.from
    property alias to: slider.to
    property alias value: slider.value
    property alias stepSize: slider.stepSize
    signal moved()

    implicitWidth: 320
    implicitHeight: Theme.touch

    function step(delta) {
        const amount = slider.stepSize > 0 ? slider.stepSize : 1
        slider.value = Math.max(slider.from,
                                Math.min(slider.to, slider.value + delta * amount))
        control.moved()
    }

    IconButton {
        id: decreaseButton
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        iconSource: "../../assets/icons/lucide/minus.svg"
        iconColor: Theme.text
        accessibleName: qsTr("Decrease")
        onClicked: control.step(-1)
    }

    Slider {
        id: slider
        anchors.left: decreaseButton.right
        anchors.right: increaseButton.left
        anchors.leftMargin: 6
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        height: Theme.touch
        snapMode: Slider.SnapAlways
        onMoved: control.moved()

        background: Rectangle {
            x: slider.leftPadding
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: slider.availableWidth
            height: 8
            radius: height / 2
            color: Theme.surfaceRaised
            border.width: 1
            border.color: Theme.border

            Rectangle {
                width: slider.visualPosition * parent.width
                height: parent.height
                radius: parent.radius
                color: Theme.greenDeep
            }
        }

        handle: Rectangle {
            x: slider.leftPadding + slider.visualPosition *
               (slider.availableWidth - width)
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: 32
            height: 32
            radius: width / 2
            color: slider.pressed ? Theme.controlPressed : Theme.text
            border.width: 2
            border.color: Theme.fairway
        }
    }

    IconButton {
        id: increaseButton
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        iconSource: "../../assets/icons/lucide/plus.svg"
        iconColor: Theme.text
        accessibleName: qsTr("Increase")
        onClicked: control.step(1)
    }
}
