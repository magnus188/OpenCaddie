import QtQuick
import OpenCaddie

Item {
    id: root
    height: Theme.statusHeight

    function wifiIcon() {
        if (network.connectedSignalStrength < 35)
            return "../../assets/icons/lucide/wifi-low.svg"
        if (network.connectedSignalStrength < 70)
            return "../../assets/icons/lucide/wifi-high.svg"
        return "../../assets/icons/lucide/wifi.svg"
    }

    function batteryIcon() {
        if (power.batteryPercent < 0)
            return "../../assets/icons/lucide/plug-zap.svg"
        if (power.externalPower)
            return "../../assets/icons/lucide/battery-charging.svg"
        if (power.batteryPercent < 35)
            return "../../assets/icons/lucide/battery-low.svg"
        if (power.batteryPercent < 75)
            return "../../assets/icons/lucide/battery-medium.svg"
        return "../../assets/icons/lucide/battery-full.svg"
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 12
        height: parent.height
        spacing: 2

        IconButton {
            width: 22
            height: 22
            padding: 3
            transparent: true
            iconSource: "../../assets/icons/lucide/bluetooth.svg"
            iconColor: Theme.text
            visible: bluetooth.connected
            accessibleName: qsTr("Bluetooth connected")
        }
        IconButton {
            width: 22
            height: 22
            padding: 3
            transparent: true
            iconSource: root.wifiIcon()
            iconColor: Theme.text
            visible: network.connectedSsid.length > 0
            accessibleName: qsTr("Wi-Fi connected")
        }
        Row {
            height: parent.height
            spacing: 2
            IconButton {
                width: 22
                height: 22
                padding: 3
                transparent: true
                iconSource: root.batteryIcon()
                iconColor: Theme.text
                accessibleName: power.batteryPercent >= 0
                                ? qsTr("Battery %1 percent").arg(power.batteryPercent)
                                : qsTr("External power")
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: power.batteryPercent >= 0 ? power.batteryPercent + "%" : ""
                visible: text.length > 0
                color: Theme.text
                font.family: "Inter"
                font.weight: Font.Medium
                font.pixelSize: Theme.px(10)
            }
        }
    }
}
