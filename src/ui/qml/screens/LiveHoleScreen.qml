import QtQuick
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    backgroundColor: Theme.focusBackground
    property bool drawerOpen: false
    property real swipeStartX: 0

    IconButton {
        id: menuButton
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 10
        anchors.topMargin: 6
        transparent: true
        iconSource: "../../assets/icons/lucide/menu.svg"
        iconColor: Theme.text
        accessibleName: qsTr("Round menu")
        onClicked: root.drawerOpen = true
        z: 10
    }

    Item {
        id: informationArea
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 500

        Item {
            id: holeHeader
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 72
            anchors.topMargin: 7
            width: 376
            height: 76

            Text {
                anchors.left: parent.left
                anchors.top: parent.top
                text: qsTr("HOLE")
                color: Theme.textMuted
                font.family: "Inter"
                font.weight: Font.DemiBold
                font.letterSpacing: 1.6
                font.pixelSize: Theme.px(10)
            }
            Text {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.topMargin: 12
                text: app.currentHole
                color: Theme.amber
                font.family: "Inter"
                font.weight: Font.Bold
                font.pixelSize: Theme.px(50)
            }
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 82
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 7
                text: qsTr("PAR %1  •  INDEX %2").arg(app.par).arg(app.strokeIndex)
                color: Theme.text
                font.family: "Inter"
                font.weight: Font.DemiBold
                font.letterSpacing: 0.7
                font.pixelSize: Theme.px(13)
            }
        }

        Column {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 72
            anchors.topMargin: 94
            width: 268
            spacing: -2

            Text {
                width: parent.width
                height: 72
                text: app.distanceText(app.backDistance)
                color: Theme.text
                font.family: "Inter"
                font.weight: Font.DemiBold
                font.pixelSize: Theme.px(42)
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                width: parent.width
                height: 92
                text: app.distanceText(app.centreDistance)
                color: Theme.fairway
                font.family: "Inter"
                font.weight: Font.Bold
                font.pixelSize: Theme.px(64)
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                Behavior on color { ColorAnimation { duration: Theme.motion } }
            }
            Text {
                width: parent.width
                height: 72
                text: app.distanceText(app.frontDistance)
                color: Theme.text
                font.family: "Inter"
                font.weight: Font.DemiBold
                font.pixelSize: Theme.px(42)
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
            }
        }

        Text {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 72
            anchors.bottomMargin: 32
            width: 268
            text: app.clubAdvice.length > 0 ? app.clubAdvice : "—"
            color: app.clubAdvice.length > 0 ? Theme.fairway : Theme.textMuted
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(34)
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideRight
        }

        Item {
            anchors.fill: parent
            anchors.leftMargin: 60

            DragHandler {
                target: null
                xAxis.enabled: true
                yAxis.enabled: false
                onActiveChanged: {
                    if (active) root.swipeStartX = centroid.position.x
                    else if (translation.x > 70) app.previousHole()
                    else if (translation.x < -70) app.nextHole()
                }
            }
        }
    }

    Rectangle {
        id: mapPreview
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 306
        color: "transparent"
        radius: 148
        clip: true

        RadialMapGlow {
            anchors.fill: parent
            innerColor: app.darkMode ? "rgba(47,203,99,0.14)"
                                     : "rgba(22,123,67,0.12)"
            middleColor: app.darkMode ? "rgba(23,54,34,0.08)"
                                      : "rgba(47,203,99,0.04)"
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

        Text {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 30
            anchors.bottomMargin: 8
            text: "© OpenStreetMap contributors"
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(7)
            opacity: 0.78
        }

        TapHandler {
            gesturePolicy: TapHandler.ReleaseWithinBounds
            onTapped: app.screen = "RoundMapScreen"
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.scrim
        opacity: root.drawerOpen ? 1 : 0
        visible: opacity > 0
        enabled: root.drawerOpen
        z: 20
        Behavior on opacity { NumberAnimation { duration: Theme.motionSheet } }
        TapHandler { onTapped: root.drawerOpen = false }
    }

    Rectangle {
        id: drawer
        x: root.drawerOpen ? 0 : -width
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 300
        color: Theme.surface
        border.width: 1
        border.color: Theme.border
        z: 21
        Behavior on x { NumberAnimation { duration: Theme.motionSheet; easing.type: Easing.OutCubic } }

        Column {
            anchors.fill: parent
            anchors.margins: Theme.gutter
            spacing: 4

            Row {
                width: parent.width
                height: 58
                Text {
                    width: parent.width - closeDrawer.width
                    height: parent.height
                    text: app.courseName
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(20)
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                IconButton {
                    id: closeDrawer
                    anchors.verticalCenter: parent.verticalCenter
                    transparent: true
                    iconSource: "../../assets/icons/lucide/x.svg"
                    iconColor: Theme.text
                    accessibleName: qsTr("Close menu")
                    onClicked: root.drawerOpen = false
                }
            }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            AppButton { width: parent.width; text: qsTr("Enter score"); variant: "surface"; onClicked: { root.drawerOpen = false; app.screen = "HoleScoreScreen" } }
            AppButton { width: parent.width; text: qsTr("Scorecard"); variant: "surface"; onClicked: { root.drawerOpen = false; app.screen = "ScorecardScreen" } }
            Item { width: 1; height: 54 }
            Rectangle { width: parent.width; height: 1; color: Theme.divider }
            AppButton { width: parent.width; text: qsTr("Finish round"); variant: "accent"; onClicked: finishDialog.open() }
            AppButton { width: parent.width; text: qsTr("Abandon round"); variant: "danger"; onClicked: abandonDialog.open() }
        }
    }

    ConfirmDialog {
        id: finishDialog
        title: qsTr("Finish round?")
        bodyText: qsTr("The round will be saved to history.")
        onConfirmed: app.finishRound()
    }
    ConfirmDialog {
        id: abandonDialog
        title: qsTr("Abandon round?")
        bodyText: qsTr("The active round will be closed.")
        onConfirmed: app.abandonRound()
    }
}
