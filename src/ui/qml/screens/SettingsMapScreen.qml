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
        title: qsTr("Map appearance")
        onBack: app.screen = "SettingsScreen"
    }

    Grid {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: header.bottom
        width: 560
        columns: 4
        spacing: 12

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
                width: 131
                height: 118
                radius: Theme.radius
                color: modelData.colors[colorIndex]
                border.width: 2
                border.color: Theme.border
                Component.onCompleted: {
                    var selected = String(
                        app.mapColors[modelData.key] || "").toUpperCase()
                    for (var index = 0;
                         index < modelData.colors.length; ++index) {
                        if (String(modelData.colors[index]).toUpperCase() ===
                                selected) {
                            colorIndex = index
                            break
                        }
                    }
                }
                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    color: "#FFFFFF"
                    style: Text.Outline
                    styleColor: "#66000000"
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(14)
                }
                TapHandler {
                    onTapped: {
                        parent.colorIndex =
                            (parent.colorIndex + 1) % modelData.colors.length
                        app.setMapColor(modelData.key,
                                        modelData.colors[parent.colorIndex])
                    }
                }
            }
        }
    }
}
