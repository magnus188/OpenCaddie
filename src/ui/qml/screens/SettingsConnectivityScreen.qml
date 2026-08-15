import QtQuick
import OpenCaddie

Item {
    id: root
    anchors.fill: parent

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Connectivity and storage")
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: header.bottom
        width: 560
        spacing: 10

        AppButton {
            width: parent.width
            text: network.connectedSsid.length > 0
                  ? qsTr("Wi-Fi: %1").arg(network.connectedSsid)
                  : qsTr("Configure Wi-Fi")
            onClicked: app.navigateTo("WifiScreen")
        }

        SectionCard {
            width: parent.width
            height: 126
            title: qsTr("OpenGolfMap server")
            AppTextField {
                anchors.fill: parent
                text: app.openGolfMapServer
                placeholderText: "https://maps.example"
                onEditingFinished: app.openGolfMapServer = text
            }
        }

        SectionCard {
            width: parent.width
            height: 115
            title: qsTr("Offline course cache")
            AppSlider {
                anchors.left: parent.left
                anchors.right: cacheValue.left
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                from: 128
                to: 4096
                stepSize: 128
                value: app.cacheLimitMb
                onMoved: app.cacheLimitMb = value
            }
            Text {
                id: cacheValue
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 78
                text: app.cacheLimitMb + " MB"
                color: Theme.text
                font.family: "Inter"
                font.pixelSize: Theme.px(12)
            }
        }

        Text {
            width: parent.width
            text: app.openGolfMapReachable
                  ? qsTr("OpenGolfMap connected · offline maps ready after download")
                  : network.internetReachable
                    ? qsTr("OpenGolfMap server unavailable")
                    : qsTr("Offline mode active")
            color: app.openGolfMapReachable ? Theme.fairway : Theme.amber
            font.family: "Inter"
            font.pixelSize: Theme.px(12)
        }
    }

}
