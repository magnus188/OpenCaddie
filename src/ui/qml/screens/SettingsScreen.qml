import QtQuick
import OpenCaddie

Item {
    anchors.fill: parent

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Settings")
        onBack: app.screen = "WelcomeScreen"
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: header.bottom
        anchors.topMargin: 1
        width: 572
        spacing: 0

        Repeater {
            model: [
                {
                    title: qsTr("Display and language"),
                    screen: "SettingsDisplayScreen"
                },
                {
                    title: qsTr("Round and scoring"),
                    screen: "SettingsRoundScreen"
                },
                {
                    title: qsTr("Map appearance"),
                    screen: "SettingsMapScreen"
                },
                {
                    title: qsTr("Connectivity and storage"),
                    screen: "SettingsConnectivityScreen"
                },
                {
                    title: qsTr("Data sources"),
                    screen: "SettingsIntegrationsScreen"
                },
                {
                    title: qsTr("Privacy and diagnostics"),
                    screen: "SettingsPrivacyScreen"
                }
            ]

            Rectangle {
                required property var modelData
                width: parent.width
                height: 58
                radius: 0
                color: settingTap.pressed ? Theme.controlPressed : "transparent"
                border.width: 0

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 16
                    text: modelData.title
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.Medium
                    font.pixelSize: Theme.px(16)
                }
                IconButton {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    transparent: true
                    iconSource: "../../assets/icons/lucide/chevron-right.svg"
                    iconColor: Theme.fairway
                    accessibleName: modelData.title
                    onClicked: app.screen = modelData.screen
                }
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: Theme.divider
                }
                TapHandler {
                    id: settingTap
                    onTapped: app.screen = modelData.screen
                }
            }
        }
    }
}
