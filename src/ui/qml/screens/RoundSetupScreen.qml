import QtQuick
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    property bool multiplayer: false
    property int holes: 18
    property bool stableford: false
    property string tee: "Yellow"
    property bool importAnalysis: app.selectedCourseHasAnalysis

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: app.selectedCourseName
        onBack: app.screen = "CourseLibraryScreen"
    }

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: startButton.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        anchors.bottomMargin: 10
        spacing: 8

        Row {
            id: playersRow
            width: parent.width
            height: 70
            spacing: 10

            Repeater {
                model: [
                    { label: qsTr("Single player"), multiplayer: false, badge: "1" },
                    { label: qsTr("Multiplayer"), multiplayer: true, badge: "V2" }
                ]

                Rectangle {
                    id: playerChoice
                    required property var modelData
                    width: (playersRow.width - playersRow.spacing) / 2
                    height: parent.height
                    radius: Theme.sheetRadius
                    color: root.multiplayer === modelData.multiplayer
                           ? Qt.rgba(0.18, 0.80, 0.39, app.darkMode ? 0.11 : 0.08)
                           : playerTap.pressed ? Theme.controlPressed : Theme.surface
                    border.width: 1
                    border.color: root.multiplayer === modelData.multiplayer
                                  ? Theme.fairway : Theme.border

                    Behavior on color {
                        ColorAnimation { duration: Theme.motionFast }
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        width: 40
                        height: 40
                        radius: 20
                        color: root.multiplayer === playerChoice.modelData.multiplayer
                               ? Theme.greenDeep : Theme.surfaceRaised

                        Text {
                            anchors.centerIn: parent
                            text: playerChoice.modelData.badge
                            color: root.multiplayer === playerChoice.modelData.multiplayer
                                   ? "#F7F8F2" : Theme.textMuted
                            font.family: "Inter"
                            font.weight: Font.Bold
                            font.pixelSize: Theme.px(playerChoice.modelData.multiplayer
                                                   ? 11 : 18)
                        }
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 64
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: playerChoice.modelData.label
                        color: Theme.text
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(16)
                        elide: Text.ElideRight
                    }

                    TapHandler {
                        id: playerTap
                        onTapped: root.multiplayer = playerChoice.modelData.multiplayer
                    }
                }
            }
        }

        Row {
            width: parent.width
            height: 92
            spacing: 10

            Rectangle {
                width: (parent.width - parent.spacing) / 2
                height: parent.height
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                Text {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 14
                    anchors.topMargin: 10
                    text: qsTr("HOLES")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.letterSpacing: 1.1
                    font.pixelSize: Theme.px(9)
                }

                Row {
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 8
                    spacing: 4
                    AppButton {
                        width: 82
                        compact: true
                        text: "9"
                        variant: root.holes === 9 ? "primary" : "surface"
                        onClicked: root.holes = 9
                    }
                    AppButton {
                        width: 82
                        compact: true
                        text: "18"
                        variant: root.holes === 18 ? "primary" : "surface"
                        onClicked: root.holes = 18
                    }
                }
            }

            Rectangle {
                width: (parent.width - parent.spacing) / 2
                height: parent.height
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                Text {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 14
                    anchors.topMargin: 10
                    text: qsTr("FORMAT")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.letterSpacing: 1.1
                    font.pixelSize: Theme.px(9)
                }

                Row {
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 8
                    spacing: 4
                    AppButton {
                        width: 150
                        compact: true
                        text: qsTr("Stroke play")
                        variant: !root.stableford ? "primary" : "surface"
                        onClicked: root.stableford = false
                    }
                    AppButton {
                        width: 150
                        compact: true
                        text: qsTr("Stableford")
                        variant: root.stableford ? "primary" : "surface"
                        onClicked: root.stableford = true
                    }
                }
            }
        }

        Row {
            width: parent.width
            height: 92
            spacing: 10

            Rectangle {
                width: (parent.width - parent.spacing) / 2
                height: parent.height
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                Text {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 14
                    anchors.topMargin: 10
                    text: qsTr("HANDICAP")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.letterSpacing: 1.1
                    font.pixelSize: Theme.px(9)
                }

                Row {
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 8
                    spacing: 4

                    IconButton {
                        iconSource: "../../assets/icons/lucide/minus.svg"
                        iconColor: Theme.text
                        accessibleName: qsTr("Decrease handicap")
                        onClicked: app.courseHandicap = Math.max(-10,
                                                                 app.courseHandicap - 1)
                    }
                    Text {
                        width: 64
                        height: Theme.touch
                        text: app.courseHandicap
                        color: Theme.amber
                        font.family: "Inter"
                        font.weight: Font.Bold
                        font.pixelSize: Theme.px(24)
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
            }

            Rectangle {
                width: (parent.width - parent.spacing) / 2
                height: parent.height
                radius: Theme.radius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                Text {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 14
                    anchors.topMargin: 10
                    text: qsTr("TEE")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.letterSpacing: 1.1
                    font.pixelSize: Theme.px(9)
                }

                Row {
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 8
                    spacing: 4

                    Repeater {
                        model: [
                            { label: qsTr("Yellow"), value: "Yellow" },
                            { label: qsTr("White"), value: "White" },
                            { label: qsTr("Red"), value: "Red" }
                        ]
                        AppButton {
                            required property var modelData
                            width: 94
                            compact: true
                            text: modelData.label
                            variant: root.tee === modelData.value ? "primary" : "surface"
                            onClicked: root.tee = modelData.value
                        }
                    }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 52
            radius: Theme.radius
            color: root.importAnalysis
                   ? Qt.rgba(0.18, 0.80, 0.39, app.darkMode ? 0.11 : 0.08)
                   : Theme.surface
            border.width: 1
            border.color: root.importAnalysis ? Theme.fairway : Theme.border
            visible: app.selectedCourseHasAnalysis

            Column {
                anchors.left: parent.left
                anchors.right: importSwitch.left
                anchors.leftMargin: 14
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                spacing: 2

                Text {
                    text: qsTr("Use saved course analysis")
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.DemiBold
                    font.pixelSize: Theme.px(15)
                }
                Text {
                    text: qsTr("Import layups from %1 analyzed holes")
                          .arg(app.selectedCourseAnalyzedHoleCount)
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(11)
                }
            }

            Rectangle {
                id: importSwitch
                anchors.right: parent.right
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                width: 48
                height: 28
                radius: 14
                color: root.importAnalysis ? Theme.fairway : Theme.controlPressed

                Rectangle {
                    x: root.importAnalysis ? parent.width - width - 4 : 4
                    anchors.verticalCenter: parent.verticalCenter
                    width: 20
                    height: 20
                    radius: 10
                    color: root.importAnalysis ? "#101211" : Theme.textMuted
                    Behavior on x {
                        NumberAnimation { duration: Theme.motionFast }
                    }
                }
            }

            TapHandler {
                onTapped: root.importAnalysis = !root.importAnalysis
            }
        }
    }

    AppButton {
        id: startButton
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        anchors.bottomMargin: 16
        text: root.multiplayer ? qsTr("Coming in V2") : qsTr("Start round")
        variant: "primary"
        enabled: !root.multiplayer
        onClicked: app.startRound(app.selectedCourseSlug, root.holes,
                                  root.stableford, app.courseHandicap, root.tee,
                                  root.importAnalysis)
    }
}
