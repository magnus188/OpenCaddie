import QtQuick
import OpenCaddie

Item {
    id: root
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: Theme.background
    }

    // The cropped course silhouette on the right echoes the unfinished Figma
    // welcome composition without depending on a remote raster asset.
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 210
        color: "#171A18"
        radius: width / 2
        clip: true
        opacity: 0.78

        CourseMap {
            anchors.fill: parent
            anchors.leftMargin: 30
            anchors.rightMargin: -36
            anchors.topMargin: 18
            anchors.bottomMargin: 18
            modelSource: "qrc:/qt/qml/OpenCaddie/assets/demo/render-model.json"
            hole: 1
            colors: app.mapColors
            playerVisible: false
        }
    }

    Text {
        id: title
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 96
        anchors.topMargin: 28
        text: "OpenCaddie"
        color: Theme.text
        font.family: "Inter"
        font.weight: Font.Bold
        font.pixelSize: Theme.px(52)
    }

    Column {
        id: status
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 18
        anchors.topMargin: 16
        spacing: 1
        Text {
            anchors.right: parent.right
            text: network.connectedSsid.length > 0
                  ? network.connectedSsid : qsTr("Offline")
            color: network.internetReachable ? Theme.fairway : Theme.textMuted
            font.family: "Inter"
            font.weight: Font.DemiBold
            font.pixelSize: Theme.px(12)
        }
        Text {
            anchors.right: parent.right
            text: power.batteryPercent >= 0
                  ? qsTr("%1% battery").arg(power.batteryPercent)
                  : qsTr("External power")
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(10)
        }
    }

    Column {
        id: menu
        anchors.left: parent.left
        anchors.top: title.bottom
        anchors.leftMargin: 202
        anchors.topMargin: app.hasActiveRound ? 22 : 42
        width: 396
        spacing: 10

        AppButton {
            width: parent.width
            height: 58
            text: app.hasActiveRound ? qsTr("Resume round") : qsTr("Play golf")
            variant: "primary"
            font.pixelSize: Theme.px(23)
            onClicked: app.hasActiveRound
                       ? app.resumeRound()
                       : app.screen = "CourseLibraryScreen"
        }
        Text {
            width: parent.width
            height: app.hasActiveRound ? 22 : 0
            visible: app.hasActiveRound
            text: qsTr("%1 · Hole %2 of %3")
                  .arg(app.courseName).arg(app.currentHole).arg(app.holeCount)
            color: Theme.amber
            font.family: "Inter"
            font.weight: Font.DemiBold
            font.pixelSize: Theme.px(12)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Repeater {
            model: [
                { label: qsTr("History"), screen: "HistoryScreen" },
                { label: qsTr("My clubs"), screen: "BagScreen" },
                { label: qsTr("Settings"), screen: "SettingsScreen" }
            ]
            AppButton {
                required property var modelData
                width: menu.width
                height: 58
                text: modelData.label
                font.weight: Font.Normal
                font.pixelSize: Theme.px(21)
                onClicked: app.screen = modelData.screen
            }
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        text: "OpenCaddie 0.1.0 · AGPL-3.0-or-later"
        color: Theme.textMuted
        font.family: "Inter"
        font.pixelSize: Theme.px(10)
    }
}
