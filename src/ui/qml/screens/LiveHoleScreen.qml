import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    property bool statsOpen: false

    Rectangle {
        anchors.fill: parent
        color: Theme.background
    }

    Column {
        id: holeHeader
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        spacing: 0

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Hole %1").arg(app.currentHole)
            color: Theme.amber
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(50)
        }
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 186
            height: 2
            color: Theme.text
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Par %1    Index %2").arg(app.par).arg(app.strokeIndex)
            color: Theme.text
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(20)
        }
    }

    Column {
        id: distanceColumn
        anchors.left: parent.left
        anchors.leftMargin: 52
        anchors.top: parent.top
        anchors.topMargin: 132
        width: 276
        spacing: -4

        Repeater {
            model: [
                { label: qsTr("Back edge"), distance: app.backDistance,
                  color: Theme.text, size: 48 },
                { label: qsTr("Centre"), distance: app.centreDistance,
                  color: Theme.fairway, size: 62 },
                { label: qsTr("Front"), distance: app.frontDistance,
                  color: Theme.text, size: 48 }
            ]
            Row {
                required property var modelData
                width: distanceColumn.width
                height: modelData.size + 10
                Text {
                    width: 54
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.label
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.weight: Font.Medium
                    font.pixelSize: Theme.px(11)
                }
                Text {
                    width: parent.width - 54
                    anchors.verticalCenter: parent.verticalCenter
                    text: app.distanceText(modelData.distance)
                    color: modelData.color
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(modelData.size)
                    horizontalAlignment: Text.AlignRight
                }
            }
        }
    }

    Rectangle {
        id: mapWell
        anchors.right: parent.right
        anchors.rightMargin: -36
        anchors.top: parent.top
        anchors.topMargin: -6
        width: 322
        height: 442
        radius: width / 2
        color: app.darkMode ? "#1B1E1C" : "#E9EEE5"
        clip: true

        CourseMap {
            anchors.fill: parent
            anchors.leftMargin: 70
            anchors.rightMargin: 44
            anchors.topMargin: 18
            anchors.bottomMargin: 18
            modelSource: app.mapSource
            hole: app.currentHole
            colors: app.mapColors
            playerX: app.playerX
            playerY: app.playerY
            playerVisible: app.playerVisible
        }
    }

    Column {
        anchors.left: parent.left
        anchors.leftMargin: 350
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 68
        width: 160
        spacing: -2
        Text {
            width: parent.width
            text: app.clubAdvice.length > 0 ? qsTr("Club") : qsTr("Club advice")
            color: Theme.textMuted
            font.family: "Inter"
            font.weight: Font.DemiBold
            font.pixelSize: Theme.px(13)
            horizontalAlignment: Text.AlignRight
        }
        Text {
            width: parent.width
            text: app.clubAdvice.length > 0 ? app.clubAdvice : "—"
            color: app.clubAdvice.length > 0 ? Theme.fairway : Theme.textMuted
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(36)
            horizontalAlignment: Text.AlignRight
        }
        Text {
            width: parent.width
            text: app.clubAdvice.length > 0
                  ? qsTr("%1 from target").arg(app.clubDelta)
                  : qsTr("Needs fresh GPS")
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(10)
            horizontalAlignment: Text.AlignRight
        }
    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 66
        spacing: 8
        AppButton {
            text: "−"
            compact: true
            width: 48
            onClicked: app.changeStrokes(-1)
        }
        Text {
            width: 86
            height: 48
            text: qsTr("%1 strokes").arg(app.strokes)
            color: Theme.text
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(15)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        AppButton {
            text: "+"
            compact: true
            width: 48
            variant: "primary"
            onClicked: app.changeStrokes(1)
        }
        AppButton {
            text: qsTr("Stats")
            compact: true
            visible: app.showAdvancedScores
            onClicked: root.statsOpen = true
        }
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 53
        width: 320
        text: qsTr("Plays like — unavailable") + "  ·  " + app.gpsStatus +
              (app.gpsAccuracy > 0
               ? qsTr(" ±%1 m").arg(Math.round(app.gpsAccuracy)) : "")
        color: Theme.textMuted
        font.family: "Inter"
        font.pixelSize: Theme.px(9)
        elide: Text.ElideRight
    }

    BottomNav {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        onPrevious: app.previousHole()
        onNext: app.nextHole()
        onScorecard: app.screen = "ScorecardScreen"
    }

    Rectangle {
        anchors.fill: parent
        color: "#88000000"
        visible: root.statsOpen
        z: 20
        TapHandler {
            onTapped: root.statsOpen = false
        }
    }

    Rectangle {
        id: statsPanel
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 402
        color: Theme.surface
        border.width: 1
        border.color: Theme.border
        visible: root.statsOpen
        z: 21

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 10

            Row {
                width: parent.width
                Text {
                    width: parent.width - closeStats.width
                    text: qsTr("Hole %1 details").arg(app.currentHole)
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(22)
                }
                AppButton {
                    id: closeStats
                    text: qsTr("Close")
                    compact: true
                    onClicked: root.statsOpen = false
                }
            }

            Row {
                spacing: 8
                Text {
                    width: 88
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Putts")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(14)
                }
                AppButton {
                    text: "−"
                    compact: true
                    onClicked: app.changePutts(-1)
                }
                Text {
                    width: 44
                    anchors.verticalCenter: parent.verticalCenter
                    text: app.putts
                    color: Theme.text
                    horizontalAlignment: Text.AlignHCenter
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(18)
                }
                AppButton {
                    text: "+"
                    compact: true
                    onClicked: app.changePutts(1)
                }
            }

            Row {
                spacing: 8
                Text {
                    width: 88
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Penalties")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(14)
                }
                AppButton {
                    text: "−"
                    compact: true
                    onClicked: app.changePenalties(-1)
                }
                Text {
                    width: 44
                    anchors.verticalCenter: parent.verticalCenter
                    text: app.penalties
                    color: Theme.text
                    horizontalAlignment: Text.AlignHCenter
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(18)
                }
                AppButton {
                    text: "+"
                    compact: true
                    onClicked: app.changePenalties(1)
                }
            }

            Text {
                text: qsTr("Fairway")
                color: Theme.textMuted
                font.family: "Inter"
                font.pixelSize: Theme.px(13)
            }
            Row {
                spacing: 5
                Repeater {
                    model: [
                        { label: qsTr("Left"), value: "left" },
                        { label: qsTr("Centre"), value: "centre" },
                        { label: qsTr("Right"), value: "right" },
                        { label: qsTr("Missed"), value: "missed" }
                    ]
                    AppButton {
                        required property var modelData
                        text: modelData.label
                        compact: true
                        variant: app.fairway === modelData.value
                                 ? "primary" : "secondary"
                        onClicked: app.setFairway(modelData.value)
                    }
                }
            }

            Row {
                spacing: 10
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Green in regulation")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(14)
                }
                Switch {
                    checked: app.gir
                    onToggled: app.setGir(checked)
                }
            }

            Text {
                text: qsTr("Notes")
                color: Theme.textMuted
                font.family: "Inter"
                font.pixelSize: Theme.px(13)
            }
            AppTextField {
                id: notesField
                width: parent.width
                text: app.notes
                placeholderText: qsTr("Optional round note")
                onEditingFinished: app.setNotes(text)
            }

            AppButton {
                anchors.right: parent.right
                text: qsTr("Save and close")
                variant: "primary"
                onClicked: {
                    app.setNotes(notesField.text)
                    root.statsOpen = false
                }
            }
        }
    }
}
