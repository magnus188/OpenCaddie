import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    property string selectedSsid: ""
    property bool hiddenNetwork: false

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Wi-Fi")
        subtitle: network.connectedSsid.length > 0
                  ? qsTr("Connected to %1").arg(network.connectedSsid)
                  : qsTr("Not connected")
    }

    AppButton {
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: header.verticalCenter
        text: network.scanning ? qsTr("Scanning…") : qsTr("Scan")
        compact: true
        enabled: !network.scanning
        onClicked: network.scan()
    }

    Row {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 16
        anchors.topMargin: 4
        anchors.bottomMargin: KeyboardController.active
                              ? KeyboardController.keyboardHeight + 8 : 16
        spacing: 14

        SectionCard {
            width: 374
            height: parent.height
            title: qsTr("Available networks")
            ListView {
                anchors.fill: parent
                model: network.networks
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                ScrollIndicator.vertical: ScrollIndicator { }
                spacing: 0
                delegate: Rectangle {
                    required property var modelData
                    width: ListView.view.width
                    height: 55
                    color: root.selectedSsid === modelData.ssid
                           ? Qt.rgba(0.18, 0.80, 0.39, 0.11)
                           : networkTap.pressed ? Theme.controlPressed
                                                : "transparent"
                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 1
                        color: Theme.divider
                    }
                    Column {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 10
                        Text {
                            text: modelData.ssid
                            color: Theme.text
                            font.family: "Inter"
                            font.weight: Font.DemiBold
                            font.pixelSize: Theme.px(14)
                        }
                        Text {
                            text: modelData.security
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(10)
                        }
                    }
                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.signal + "%"
                        color: modelData.signal >= 60 ? Theme.fairway : Theme.amber
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(12)
                    }
                    TapHandler {
                        id: networkTap
                        onTapped: {
                            root.selectedSsid = modelData.ssid
                            ssid.text = modelData.ssid
                            password.forceActiveFocus()
                            KeyboardController.open(password, false)
                        }
                    }
                }
            }
        }

        SectionCard {
            width: parent.width - 388
            height: parent.height
            title: qsTr("Connect")
            Column {
                anchors.fill: parent
                spacing: 7
                AppTextField {
                    id: ssid
                    width: parent.width
                    placeholderText: qsTr("Network name")
                    enabled: root.hiddenNetwork
                }
                AppTextField {
                    id: password
                    width: parent.width
                    placeholderText: qsTr("Password")
                    echoMode: TextInput.Password
                }
                Row {
                    spacing: 8
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Hidden network")
                        color: Theme.text
                        font.family: "Inter"
                        font.pixelSize: Theme.px(13)
                    }
                    AppSwitch {
                        accessibleName: qsTr("Hidden network")
                        checked: root.hiddenNetwork
                        onToggled: root.hiddenNetwork = checked
                    }
                }
                Row {
                    spacing: 8
                    AppButton {
                        text: qsTr("Connect")
                        variant: "primary"
                        enabled: ssid.text.length > 0
                        onClicked: {
                            network.connectNetwork(ssid.text, password.text,
                                                   root.hiddenNetwork)
                            password.text = ""
                            KeyboardController.close()
                        }
                    }
                    AppButton {
                        text: qsTr("Forget")
                        variant: "danger"
                        enabled: ssid.text.length > 0
                        onClicked: network.forgetNetwork(ssid.text)
                    }
                }
                Text {
                    width: parent.width
                    text: qsTr("OpenCaddie supports personal WPA2/WPA3. Enterprise Wi-Fi is outside V1.")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(11)
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

}
