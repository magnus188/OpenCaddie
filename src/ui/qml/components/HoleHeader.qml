import QtQuick
import OpenCaddie

// Fixed hole identity shown above the changing live-round content.
Item {
    id: header

    property int hole: 1
    property int par: 4
    property int strokeIndex: 1
    property bool ready: false

    onHoleChanged: {
        if (ready)
            holePulse.restart()
    }
    Component.onCompleted: ready = true

    implicitWidth: 276
    implicitHeight: 82

    Row {
        anchors.centerIn: parent
        height: 68

        Item {
            width: 112
            height: parent.height

            Column {
                anchors.centerIn: parent
                spacing: -5

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Hole")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(11)
                    font.capitalization: Font.AllUppercase
                    font.letterSpacing: 1.6
                }

                Text {
                    id: holeNumber
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: header.hole
                    color: Theme.amber
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(50)
                }
            }
        }

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: 1
            height: 44
            color: Theme.divider
        }

        HeaderMetric {
            width: 81
            height: parent.height
            label: qsTr("Par")
            value: header.par
        }

        HeaderMetric {
            width: 81
            height: parent.height
            label: qsTr("Index")
            value: header.strokeIndex
        }
    }

    SequentialAnimation {
        id: holePulse
        NumberAnimation {
            target: holeNumber
            property: "scale"
            from: 0.92
            to: 1.04
            duration: Theme.motionFast
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            target: holeNumber
            property: "scale"
            from: 1.04
            to: 1
            duration: Theme.motion
            easing.type: Easing.OutBack
            easing.overshoot: 0.45
        }
    }

    component HeaderMetric: Item {
        id: metric

        property string label
        property string value

        Column {
            anchors.centerIn: parent
            spacing: 1

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: metric.label
                color: Theme.textMuted
                font.family: "Inter"
                font.weight: Font.DemiBold
                font.pixelSize: Theme.px(10)
                font.capitalization: Font.AllUppercase
                font.letterSpacing: 0.9
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: metric.value
                color: Theme.text
                font.family: "Inter"
                font.weight: Font.Bold
                font.pixelSize: Theme.px(26)
            }
        }
    }
}
