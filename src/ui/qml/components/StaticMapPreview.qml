import QtQuick
import OpenCaddie

// Read-only live-hole map. The outer view is intentionally static; tapping it
// opens the full map where pan, zoom and rotation controls live.
Rectangle {
    id: root
    signal openRequested

    color: "transparent"
    radius: Math.min(width, height) / 2
    clip: true
    Accessible.role: Accessible.Button
    Accessible.name: qsTr("Open full map")
    Accessible.onPressAction: root.openRequested()

    RadialMapGlow {
        anchors.fill: parent
        innerColor: app.darkMode ? "rgba(47,203,99,0.14)" : "rgba(22,123,67,0.12)"
        middleColor: app.darkMode ? "rgba(23,54,34,0.08)" : "rgba(47,203,99,0.04)"
    }

    CourseMap {
        id: previewMap
        anchors.fill: parent
        anchors.leftMargin: 44
        anchors.rightMargin: 12
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        modelSource: app.mapSource
        hole: app.currentHole
        colors: app.mapColors
        playerX: app.playerX
        playerY: app.playerY
        playerVisible: app.playerVisible
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.surfaceRaised
        opacity: 0.92
        visible: !previewMap.ready && previewMap.errorText.length > 0

        Text {
            anchors.centerIn: parent
            width: 180
            text: previewMap.errorText
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(13)
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }

    TapHandler {
        gesturePolicy: TapHandler.DragThreshold
        onTapped: root.openRequested()
    }

    IconButton {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 18
        anchors.topMargin: 8
        transparent: true
        iconSource: "../../assets/icons/expand.svg"
        iconColor: Theme.text
        accessibleName: qsTr("Open full map")
        onClicked: root.openRequested()
    }
}
