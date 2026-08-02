import QtQuick
import OpenCaddie

Item {
    id: control
    property alias text: label.text
    property url iconSource
    property bool active: false
    signal clicked()

    implicitWidth: 184
    implicitHeight: 58
    Accessible.role: Accessible.Button
    Accessible.name: text

    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        radius: Theme.radius
        color: control.active ? Qt.rgba(0.18, 0.8, 0.39, 0.08)
                              : tap.pressed ? Theme.controlPressed : "transparent"
        border.width: 0
    }

    Row {
        anchors.centerIn: parent
        spacing: 10

        Image {
            width: 24
            height: 24
            anchors.verticalCenter: parent.verticalCenter
            source: control.iconSource
            sourceSize: Qt.size(48, 48)
            fillMode: Image.PreserveAspectFit
            opacity: control.active ? 1.0 : 0.82
        }

        Text {
            id: label
            anchors.verticalCenter: parent.verticalCenter
            color: control.active ? Theme.fairway : Theme.text
            font.family: "Inter"
            font.weight: Font.DemiBold
            font.pixelSize: Theme.px(15)
        }
    }

    TapHandler {
        id: tap
        onTapped: control.clicked()
    }
}
