import QtQuick
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    backgroundColor: Theme.focusBackground

    Item {
        id: courseArtwork
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 270

        RadialMapGlow {
            anchors.fill: parent
            innerColor: app.darkMode ? "rgba(47,203,99,0.16)"
                                     : "rgba(22,123,67,0.14)"
            middleColor: app.darkMode ? "rgba(33,78,48,0.08)"
                                      : "rgba(47,203,99,0.05)"
        }

        CourseMap {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenterOffset: 18
            width: 220
            height: parent.height - 10
            modelSource: "qrc:/qt/qml/OpenCaddie/assets/demo/home-render-model.json"
            hole: 1
            colors: app.mapColors
            playerVisible: false
        }
    }

    Text {
        id: title
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.topMargin: 4
        text: "OpenCaddie"
        color: Theme.text
        font.family: "Inter"
        font.weight: Font.Bold
        font.pixelSize: Theme.px(46)
    }

    Column {
        id: menu
        anchors.left: parent.left
        anchors.top: title.bottom
        anchors.leftMargin: Theme.gutter
        anchors.topMargin: 8
        width: 500
        spacing: 0

        AppButton {
            width: parent.width
            height: 52
            text: app.hasActiveRound ? qsTr("Resume round") : qsTr("Play golf")
            variant: "primary"
            font.pixelSize: Theme.px(19)
            onClicked: app.hasActiveRound
                       ? app.resumeRound()
                       : app.openCoursePicker("start")
        }

        Text {
            width: parent.width
            height: app.hasActiveRound ? 22 : 6
            visible: app.hasActiveRound
            text: qsTr("%1 · Hole %2 of %3")
                  .arg(app.courseName).arg(app.currentHole).arg(app.holeCount)
            color: Theme.amber
            font.family: "Inter"
            font.weight: Font.DemiBold
            font.pixelSize: Theme.px(11)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        Repeater {
            model: [
                { label: qsTr("Course analyzer"), action: "plan" },
                { label: qsTr("History"), screen: "HistoryScreen" },
                { label: qsTr("Stats"), screen: "StatsScreen" },
                { label: qsTr("My clubs"), screen: "BagScreen" },
                { label: qsTr("Settings"), screen: "SettingsScreen" }
            ]

            Rectangle {
                id: row
                required property var modelData
                width: menu.width
                height: 51
                color: rowTap.pressed ? Theme.controlPressed : "transparent"

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: row.modelData.label
                    color: Theme.text
                    font.family: "Inter"
                    font.pixelSize: Theme.px(18)
                }
                IconButton {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    transparent: true
                    iconSource: "../../assets/icons/lucide/chevron-right.svg"
                    iconColor: Theme.fairway
                    accessibleName: row.modelData.label
                    onClicked: row.modelData.action === "plan"
                               ? app.openCoursePicker("plan")
                               : app.navigateTo(row.modelData.screen)
                }
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: Theme.divider
                }
                TapHandler {
                    id: rowTap
                    onTapped: row.modelData.action === "plan"
                              ? app.openCoursePicker("plan")
                              : app.navigateTo(row.modelData.screen)
                }
            }
        }
    }
}
