import QtQuick
import OpenCaddie

Rectangle {
    id: banner
    property int hole: app.currentHole

    width: Math.min(parent ? parent.width - 120 : 520, 520)
    height: 60
    radius: Theme.sheetRadius
    color: Theme.overlay
    border.width: 1
    border.color: Theme.fairway
    visible: opacity > 0
    opacity: 0

    function showForHole(number) {
        hole = number
        opacity = 1
        hideTimer.restart()
    }

    Row {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 8

        Text {
            width: parent.width - enterButton.width - closeButton.width - 16
            height: parent.height
            text: qsTr("Ready to score hole %1?").arg(banner.hole)
            color: Theme.text
            font.family: "Inter"
            font.weight: Font.DemiBold
            font.pixelSize: Theme.px(15)
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        AppButton {
            id: enterButton
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Enter score")
            variant: "primary"
            compact: true
            onClicked: {
                banner.opacity = 0
                app.navigateTo("HoleScoreScreen")
            }
        }

        IconButton {
            id: closeButton
            anchors.verticalCenter: parent.verticalCenter
            transparent: true
            iconSource: "../../assets/icons/lucide/x.svg"
            iconColor: Theme.text
            accessibleName: qsTr("Dismiss")
            onClicked: banner.opacity = 0
        }
    }

    Connections {
        target: app
        function onScoreEntryRequested(hole) {
            banner.showForHole(hole)
        }
    }

    Timer {
        id: hideTimer
        interval: 12000
        onTriggered: banner.opacity = 0
    }

    Behavior on opacity {
        NumberAnimation { duration: Theme.motionSheet; easing.type: Easing.OutCubic }
    }
}
