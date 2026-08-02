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
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Round and scoring")
        onBack: app.screen = "SettingsScreen"
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: header.bottom
        width: 560
        spacing: 0

        Rectangle {
            width: parent.width
            height: 64
            color: "transparent"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Handicap")
                color: Theme.text
                font.family: "Inter"
                font.pixelSize: Theme.px(14)
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                IconButton {
                    iconSource: "../../assets/icons/lucide/minus.svg"
                    iconColor: Theme.text
                    accessibleName: qsTr("Decrease handicap")
                    onClicked: app.courseHandicap = Math.max(-10,
                                                             app.courseHandicap - 1)
                }
                Text {
                    width: 58
                    height: Theme.touch
                    text: app.courseHandicap
                    color: Theme.amber
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(20)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                IconButton {
                    iconSource: "../../assets/icons/lucide/plus.svg"
                    iconColor: Theme.text
                    accessibleName: qsTr("Increase handicap")
                    onClicked: app.courseHandicap = Math.min(54,
                                                             app.courseHandicap + 1)
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.divider
            }
        }

        Repeater {
            model: [
                {
                    label: qsTr("Advanced score fields"),
                    checked: app.showAdvancedScores,
                    key: "advanced"
                },
                {
                    label: qsTr("Automatic hole advance"),
                    checked: app.automaticHoleAdvance,
                    key: "advance"
                },
                {
                    label: qsTr("Subtle score celebrations"),
                    checked: app.celebrationsEnabled,
                    key: "celebrations"
                }
            ]
            Rectangle {
                required property var modelData
                width: parent.width
                height: 56
                color: "transparent"
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: Theme.divider
                }
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.label
                    color: Theme.text
                    font.family: "Inter"
                    font.pixelSize: Theme.px(14)
                }
                Switch {
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    checked: modelData.checked
                    onToggled: {
                        if (modelData.key === "advanced")
                            app.showAdvancedScores = checked
                        else if (modelData.key === "advance")
                            app.automaticHoleAdvance = checked
                        else
                            app.celebrationsEnabled = checked
                    }
                }
            }
        }

        SectionCard {
            width: parent.width
            height: 105
            title: qsTr("Club advice bias")
            Slider {
                anchors.left: parent.left
                anchors.right: biasValue.left
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                from: app.metric ? -20 : -22
                to: app.metric ? 20 : 22
                stepSize: 1
                value: app.recommendationBias
                onMoved: app.recommendationBias = value
            }
            Text {
                id: biasValue
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 70
                text: Math.round(app.recommendationBias) +
                      (app.metric ? " m" : " yd")
                color: Theme.text
                font.family: "Inter"
                font.pixelSize: Theme.px(13)
            }
        }
    }
}
