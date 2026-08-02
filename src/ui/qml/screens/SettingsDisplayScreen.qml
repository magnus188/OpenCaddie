import QtQuick
import QtQuick.Controls
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
        title: qsTr("Display and language")
        onBack: app.screen = "SettingsScreen"
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: header.bottom
        width: 560
        spacing: 12

        Row {
            width: parent.width
            spacing: 8
            AppButton {
                width: 132
                text: qsTr("Dark")
                variant: app.darkMode ? "primary" : "secondary"
                onClicked: app.darkMode = true
            }
            AppButton {
                width: 132
                text: qsTr("Light")
                variant: !app.darkMode ? "primary" : "secondary"
                onClicked: app.darkMode = false
            }
            AppButton {
                width: 132
                text: app.metric ? qsTr("Metric") : qsTr("Imperial")
                onClicked: app.metric = !app.metric
            }
            AppButton {
                width: 132
                text: app.language === "en" ? "English" : "Norsk"
                onClicked: app.language = app.language === "en" ? "nb" : "en"
            }
        }

        SectionCard {
            width: parent.width
            height: 92
            title: qsTr("Text size")
            Slider {
                anchors.left: parent.left
                anchors.right: valueLabel.left
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                from: 0.8
                to: 1.5
                value: app.textScale
                stepSize: 0.1
                onMoved: app.textScale = value
            }
            Text {
                id: valueLabel
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 55
                text: Math.round(app.textScale * 100) + "%"
                color: Theme.text
                font.family: "Inter"
                font.pixelSize: Theme.px(13)
            }
        }

        SectionCard {
            width: parent.width
            height: 92
            title: qsTr("Brightness")
            Slider {
                anchors.left: parent.left
                anchors.right: brightnessValue.left
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                from: 10
                to: 100
                value: app.brightness
                stepSize: 5
                onMoved: app.brightness = value
            }
            Text {
                id: brightnessValue
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 55
                text: app.brightness + "%"
                color: Theme.text
                font.family: "Inter"
                font.pixelSize: Theme.px(13)
            }
        }
    }
}
