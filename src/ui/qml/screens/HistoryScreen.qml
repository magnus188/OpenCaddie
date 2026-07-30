import QtQuick
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    property string selectedRoundId: ""

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        title: qsTr("Round history")
        subtitle: qsTr("Search, inspect, resume, and export")
        onBack: app.screen = "WelcomeScreen"
    }

    Row {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 16
        anchors.topMargin: 4
        spacing: 14

        SectionCard {
            width: 340
            height: parent.height
            title: qsTr("Rounds")

            AppTextField {
                id: search
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                placeholderText: qsTr("Search course")
                onTextChanged: app.refreshHistory(text)
            }

            ListView {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: search.bottom
                anchors.bottom: parent.bottom
                anchors.topMargin: 8
                model: app.history
                clip: true
                spacing: 6

                delegate: Rectangle {
                    required property var modelData
                    required property int index
                    width: ListView.view.width
                    height: 64
                    radius: Theme.radius
                    color: root.selectedRoundId === modelData.id
                           ? Qt.rgba(0.18, 0.80, 0.39, 0.14)
                           : Theme.surfaceRaised
                    border.width: 1
                    border.color: root.selectedRoundId === modelData.id
                                  ? Theme.fairway : Theme.border
                    Column {
                        anchors.left: parent.left
                        anchors.right: status.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: 10
                        spacing: 2
                        Text {
                            width: parent.width
                            text: modelData.courseName
                            color: Theme.text
                            font.family: "Inter"
                            font.weight: Font.DemiBold
                            font.pixelSize: Theme.px(14)
                            elide: Text.ElideRight
                        }
                        Text {
                            width: parent.width
                            text: modelData.startedAt
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(11)
                            elide: Text.ElideRight
                        }
                    }
                    Text {
                        id: status
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.status
                        color: modelData.status === "completed"
                               ? Theme.fairway : Theme.amber
                        font.family: "Inter"
                        font.pixelSize: Theme.px(11)
                    }
                    TapHandler {
                        onTapped: {
                            root.selectedRoundId = modelData.id
                            app.selectHistoryRound(modelData.id)
                        }
                    }
                }
            }
        }

        SectionCard {
            width: parent.width - 354
            height: parent.height
            title: app.roundDetail.courseName || qsTr("Select a round")
            subtitle: app.roundDetail.startedAt || qsTr("Score and statistics")

            ListView {
                id: detailList
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: actions.top
                anchors.bottomMargin: 8
                model: app.roundDetail.scores || []
                clip: true
                spacing: 3

                header: Row {
                    width: ListView.view.width
                    height: 30
                    Repeater {
                        model: [
                            { text: qsTr("Hole"), width: 54 },
                            { text: qsTr("Par"), width: 48 },
                            { text: qsTr("Score"), width: 64 },
                            { text: qsTr("Putts"), width: 58 },
                            { text: qsTr("Pen."), width: 50 },
                            { text: qsTr("Fairway"), width: 90 }
                        ]
                        Text {
                            required property var modelData
                            width: modelData.width
                            height: parent.height
                            text: modelData.text
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(11)
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                delegate: Rectangle {
                    required property var modelData
                    width: ListView.view.width
                    height: 34
                    radius: 4
                    color: index % 2 ? Theme.surfaceRaised : "transparent"
                    Row {
                        anchors.fill: parent
                        Repeater {
                            model: [
                                { text: modelData.hole, width: 54 },
                                { text: modelData.par, width: 48 },
                                { text: modelData.strokes, width: 64 },
                                { text: modelData.putts || "—", width: 58 },
                                { text: modelData.penalties || "—", width: 50 },
                                { text: modelData.fairway || "—", width: 90 }
                            ]
                            Text {
                                required property var modelData
                                width: modelData.width
                                height: parent.height
                                text: modelData.text
                                color: Theme.text
                                font.family: "Inter"
                                font.pixelSize: Theme.px(12)
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }
            }

            Row {
                id: actions
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                spacing: 7
                AppButton {
                    text: qsTr("JSON")
                    compact: true
                    enabled: root.selectedRoundId.length > 0
                    onClicked: app.exportRound(root.selectedRoundId, "json")
                }
                AppButton {
                    text: qsTr("CSV")
                    compact: true
                    enabled: root.selectedRoundId.length > 0
                    onClicked: app.exportRound(root.selectedRoundId, "csv")
                }
                Item { width: parent.width - 248; height: 1 }
                AppButton {
                    text: qsTr("Delete")
                    compact: true
                    variant: "danger"
                    enabled: root.selectedRoundId.length > 0
                    onClicked: deleteDialog.open()
                }
            }
        }
    }

    ConfirmDialog {
        id: deleteDialog
        title: qsTr("Delete round?")
        bodyText: qsTr("This permanently removes the local scorecard and statistics.")
        onConfirmed: {
            if (app.deleteRound(root.selectedRoundId)) {
                root.selectedRoundId = ""
                app.selectHistoryRound("")
            }
        }
    }
}
