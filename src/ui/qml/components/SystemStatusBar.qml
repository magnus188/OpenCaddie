import QtQuick
import OpenCaddie

Item {
    id: root
    height: Theme.statusHeight

    function themedIcon(name) {
        return "../../assets/icons/lucide/" + name +
               (app.darkMode ? "-dark.svg" : ".svg")
    }

    function wifiIcon() {
        if (network.connectedSignalStrength < 35)
            return themedIcon("wifi-low")
        if (network.connectedSignalStrength < 70)
            return themedIcon("wifi-high")
        return themedIcon("wifi")
    }

    function batteryIcon() {
        if (power.batteryPercent < 0)
            return themedIcon("plug-zap")
        if (power.externalPower)
            return themedIcon("battery-charging")
        if (power.batteryPercent < 35)
            return themedIcon("battery-low")
        if (power.batteryPercent < 75)
            return themedIcon("battery-medium")
        return themedIcon("battery-full")
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 12
        height: parent.height
        spacing: 2

        Image {
            width: 22
            height: 22
            source: root.themedIcon("bluetooth")
            sourceSize: Qt.size(16, 16)
            fillMode: Image.Pad
            visible: bluetooth.connected
            Accessible.role: Accessible.StaticText
            Accessible.name: qsTr("Bluetooth connected")
        }
        Image {
            width: 22
            height: 22
            source: root.wifiIcon()
            sourceSize: Qt.size(16, 16)
            fillMode: Image.Pad
            visible: network.connectedSsid.length > 0
            Accessible.role: Accessible.StaticText
            Accessible.name: qsTr("Wi-Fi connected")
        }
        Row {
            height: parent.height
            spacing: 2
            Image {
                width: 22
                height: 22
                source: root.batteryIcon()
                sourceSize: Qt.size(16, 16)
                fillMode: Image.Pad
                Accessible.role: Accessible.StaticText
                Accessible.name: power.batteryPercent >= 0
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
