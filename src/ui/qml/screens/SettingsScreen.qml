import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    id: root
    anchors.fill: parent

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        title: qsTr("Settings")
        subtitle: qsTr("Display, scoring, map, connectivity, and privacy")
        onBack: app.screen = "WelcomeScreen"
    }

    Flickable {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 16
        anchors.topMargin: 4
        contentWidth: width
        contentHeight: settingsGrid.height
        clip: true

        Grid {
            id: settingsGrid
            width: parent.width
            columns: 2
            columnSpacing: 14
            rowSpacing: 14

            SectionCard {
                width: (settingsGrid.width - 14) / 2
                height: 170
                title: qsTr("Display")
                Column {
                    anchors.fill: parent
                    spacing: 6
                    Row {
                        spacing: 6
                        AppButton {
                            text: qsTr("Dark")
                            compact: true
                            variant: app.darkMode ? "primary" : "secondary"
                            onClicked: app.darkMode = true
                        }
                        AppButton {
                            text: qsTr("Light")
                            compact: true
                            variant: !app.darkMode ? "primary" : "secondary"
                            onClicked: app.darkMode = false
                        }
                        AppButton {
                            text: app.metric ? qsTr("Metric") : qsTr("Imperial")
                            compact: true
                            onClicked: app.metric = !app.metric
                        }
                    }
                    Row {
                        spacing: 10
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Text size")
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(12)
                        }
                        Slider {
                            width: 180
                            from: 0.8
                            to: 1.5
                            value: app.textScale
                            stepSize: 0.1
                            onMoved: app.textScale = value
                        }
                    }
                    Row {
                        spacing: 10
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Brightness")
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(12)
                        }
                        Slider {
                            width: 170
                            from: 10
                            to: 100
                            value: power.brightness
                            stepSize: 5
                            onMoved: power.brightness = value
                        }
                    }
                }
            }

            SectionCard {
                width: (settingsGrid.width - 14) / 2
                height: 220
                title: qsTr("Round behavior")
                Column {
                    anchors.fill: parent
                    spacing: 7
                    Row {
                        spacing: 8
                        Text {
                            width: 230
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Advanced score fields")
                            color: Theme.text
                            font.family: "Inter"
                            font.pixelSize: Theme.px(13)
                        }
                        Switch {
                            checked: app.showAdvancedScores
                            onToggled: app.showAdvancedScores = checked
                        }
                    }
                    Row {
                        spacing: 8
                        Text {
                            width: 230
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Automatic hole advance")
                            color: Theme.text
                            font.family: "Inter"
                            font.pixelSize: Theme.px(13)
                        }
                        Switch {
                            checked: app.automaticHoleAdvance
                            onToggled: app.automaticHoleAdvance = checked
                        }
                    }
                    Row {
                        spacing: 6
                        AppButton {
                            text: "English"
                            compact: true
                            variant: app.language === "en" ? "primary" : "secondary"
                            onClicked: app.language = "en"
                        }
                        AppButton {
                            text: "Norsk"
                            compact: true
                            variant: app.language === "nb" ? "primary" : "secondary"
                            onClicked: app.language = "nb"
                        }
                    }
                    Row {
                        spacing: 8
                        Text {
                            width: 135
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Club advice bias")
                            color: Theme.text
                            font.family: "Inter"
                            font.pixelSize: Theme.px(12)
                        }
                        Slider {
                            width: 135
                            from: app.metric ? -20 : -22
                            to: app.metric ? 20 : 22
                            stepSize: 1
                            value: app.recommendationBias
                            onMoved: app.recommendationBias = value
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: Math.round(app.recommendationBias) +
                                  (app.metric ? " m" : " yd")
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(11)
                        }
                    }
                }
            }

            SectionCard {
                width: (settingsGrid.width - 14) / 2
                height: 190
                title: qsTr("Map colors")
                subtitle: qsTr("Tap a swatch to cycle accessible presets")

                Grid {
                    anchors.fill: parent
                    columns: 4
                    spacing: 8
                    Repeater {
                        model: [
                            { key: "fairway", label: qsTr("Fairway"),
                              colors: ["#2FCB63", "#54A84F", "#7FBF4D"] },
                            { key: "green", label: qsTr("Green"),
                              colors: ["#8ED66B", "#91D66F", "#B1D96B"] },
                            { key: "water", label: qsTr("Water"),
                              colors: ["#2BA7D7", "#358FC4", "#267CCB"] },
                            { key: "bunker", label: qsTr("Sand"),
                              colors: ["#E0C27A", "#DFC98E", "#D6AC63"] },
                            { key: "rough", label: qsTr("Rough"),
                              colors: ["#315C35", "#416A3E", "#2B5140"] },
                            { key: "wood", label: qsTr("Wood"),
                              colors: ["#1A5B35", "#225B43", "#284E31"] },
                            { key: "tee", label: qsTr("Tee"),
                              colors: ["#70B85B", "#77C45F", "#86B769"] },
                            { key: "path", label: qsTr("Path"),
                              colors: ["#8B8174", "#A19687", "#736B62"] }
                        ]
                        Rectangle {
                            required property var modelData
                            property int colorIndex: 0
                            width: 78
                            height: 58
                            radius: Theme.radius
                            color: modelData.colors[colorIndex]
                            border.width: 2
                            border.color: Theme.border
                            Text {
                                anchors.centerIn: parent
                                text: modelData.label
                                color: "#FFFFFF"
                                style: Text.Outline
                                styleColor: "#66000000"
                                font.family: "Inter"
                                font.weight: Font.DemiBold
                                font.pixelSize: Theme.px(11)
                            }
                            TapHandler {
                                onTapped: {
                                    parent.colorIndex =
                                        (parent.colorIndex + 1) %
                                        modelData.colors.length
                                    app.setMapColor(
                                        modelData.key,
                                        modelData.colors[parent.colorIndex])
                                }
                            }
                        }
                    }
                }
            }

            SectionCard {
                width: (settingsGrid.width - 14) / 2
                height: 270
                title: qsTr("Connectivity and diagnostics")
                Column {
                    anchors.fill: parent
                    spacing: 8
                    AppButton {
                        text: qsTr("Wi-Fi settings")
                        onClicked: app.screen = "WifiScreen"
                    }
                    AppTextField {
                        id: server
                        width: parent.width
                        text: app.openGolfMapServer
                        placeholderText: "https://maps.example"
                        onEditingFinished: app.openGolfMapServer = text
                    }
                    Text {
                        width: parent.width
                        text: qsTr("GPS: %1").arg(app.gpsStatus) + " · " +
                              (network.internetReachable
                               ? qsTr("Internet available") : qsTr("Offline"))
                        color: Theme.textMuted
                        font.family: "Inter"
                        font.pixelSize: Theme.px(11)
                        wrapMode: Text.WordWrap
                    }
                    Row {
                        spacing: 8
                        Text {
                            width: 120
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Course cache")
                            color: Theme.text
                            font.family: "Inter"
                            font.pixelSize: Theme.px(12)
                        }
                        Slider {
                            width: 150
                            from: 128
                            to: 4096
                            stepSize: 128
                            value: app.cacheLimitMb
                            onMoved: app.cacheLimitMb = value
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: app.cacheLimitMb + " MB"
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(11)
                        }
                    }
                    Row {
                        spacing: 8
                        Text {
                            width: 230
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Privacy-safe diagnostic logging")
                            color: Theme.text
                            font.family: "Inter"
                            font.pixelSize: Theme.px(12)
                        }
                        Switch {
                            checked: app.diagnosticLogging
                            onToggled: app.diagnosticLogging = checked
                        }
                    }
                    AppButton {
                        text: qsTr("Reset settings")
                        variant: "danger"
                        compact: true
                        onClicked: resetDialog.open()
                    }
                }
            }
        }
    }

    ConfirmDialog {
        id: resetDialog
        title: qsTr("Reset settings?")
        bodyText: qsTr("Round history, clubs, and downloaded courses are kept.")
        onConfirmed: app.resetSettings()
    }
}
