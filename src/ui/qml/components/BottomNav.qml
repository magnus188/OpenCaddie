import QtQuick
import OpenCaddie

Item {
    id: nav
    signal previous()
    signal next()
    signal scorecard()

    implicitHeight: 58

    AppButton {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        text: qsTr("Previous")
        compact: true
        enabled: app.currentHole > 1
        onClicked: nav.previous()
    }

    Row {
        anchors.centerIn: parent
        spacing: 8
        Repeater {
            model: 3
            Rectangle {
                required property int index
                width: index === 1 ? 34 : 10
                height: 10
                radius: 5
                color: index === 1 ? Theme.fairway : Theme.border
            }
        }
    }

    AppButton {
        anchors.right: nextButton.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        text: qsTr("Scorecard")
        compact: true
        onClicked: nav.scorecard()
    }

    AppButton {
        id: nextButton
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        text: app.currentHole === app.holeCount ? qsTr("Finish") : qsTr("Next")
        variant: "primary"
        compact: true
        onClicked: nav.next()
    }
}

