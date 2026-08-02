import QtQuick
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    property int grossTotal: {
        var total = 0
        for (var i = 0; i < app.scorecard.length; ++i)
            total += app.scorecard[i].strokes
        return total
    }
    property int parTotal: {
        var total = 0
        for (var i = 0; i < app.scorecard.length; ++i)
            if (app.scorecard[i].strokes > 0)
                total += app.scorecard[i].par
        return total
    }
    property int pointsTotal: {
        var total = 0
        for (var i = 0; i < app.scorecard.length; ++i)
            total += app.scorecard[i].stableford
        return total
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Scorecard")
        subtitle: app.courseName
        onBack: app.screen = "LiveHoleScreen"
    }

    Row {
        id: summary
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: header.verticalCenter
        spacing: 8
        Item {
            width: 82; height: 42
            Text {
                anchors.centerIn: parent
                text: qsTr("Gross %1").arg(root.grossTotal)
                color: Theme.text
                font.family: "Inter"; font.weight: Font.DemiBold
                font.pixelSize: Theme.px(13)
            }
        }
        Item {
            width: 82; height: 42
            Text {
                anchors.centerIn: parent
                text: root.grossTotal === 0 ? qsTr("E") :
                      (root.grossTotal - root.parTotal > 0 ? "+" : "") +
                      (root.grossTotal - root.parTotal)
                color: Theme.amber
                font.family: "Inter"; font.weight: Font.Bold
                font.pixelSize: Theme.px(14)
            }
        }
        Item {
            width: 82; height: 42
            Text {
                anchors.centerIn: parent
                text: qsTr("%1 pts").arg(root.pointsTotal)
                color: Theme.fairway
                font.family: "Inter"; font.weight: Font.Bold
                font.pixelSize: Theme.px(14)
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: footer.top
        anchors.margins: 16
        anchors.topMargin: 4
        anchors.bottomMargin: 8
        color: "transparent"

        ListView {
            anchors.fill: parent
            anchors.margins: 0
            clip: true
            spacing: 2
            model: app.scorecard
            header: Item {
                width: ListView.view.width
                height: 34
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: Theme.divider
                }
                Row {
                    anchors.fill: parent
                    Repeater {
                        model: [
                            { text: qsTr("Hole"), width: 70 },
                            { text: qsTr("Par"), width: 70 },
                            { text: qsTr("Index"), width: 70 },
                            { text: qsTr("Strokes"), width: 120 },
                            { text: qsTr("Putts"), width: 100 },
                            { text: qsTr("Pen."), width: 90 },
                            { text: qsTr("Points"), width: 100 }
                        ]
                        Text {
                            required property var modelData
                            width: modelData.width
                            height: parent.height
                            text: modelData.text
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.weight: Font.DemiBold
                            font.pixelSize: Theme.px(12)
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            delegate: Rectangle {
                required property var modelData
                required property int index
                width: ListView.view.width
                height: 38
                color: modelData.hole === app.currentHole
                       ? Qt.rgba(0.18, 0.80, 0.39, 0.12)
                       : scoreTap.pressed ? Theme.controlPressed : "transparent"
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: Theme.divider
                }
                Row {
                    anchors.fill: parent
                    Repeater {
                        model: [
                            { text: modelData.hole, width: 70 },
                            { text: modelData.par, width: 70 },
                            { text: modelData.index, width: 70 },
                            { text: modelData.strokes || "—", width: 120 },
                            { text: modelData.putts || "—", width: 100 },
                            { text: modelData.penalties || "—", width: 90 },
                            { text: modelData.stableford || "—", width: 100 }
                        ]
                        Text {
                            required property var modelData
                            width: modelData.width
                            height: parent.height
                            text: modelData.text
                            color: Theme.text
                            font.family: "Inter"
                            font.weight: Font.Medium
                            font.pixelSize: Theme.px(13)
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
                TapHandler {
                    id: scoreTap
                    onTapped: {
                        app.setHole(modelData.hole)
                        app.screen = "LiveHoleScreen"
                    }
                }
            }
        }
    }

    Row {
        id: footer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16
        height: 48
        AppButton {
            text: qsTr("Back to hole")
            onClicked: app.screen = "LiveHoleScreen"
        }
        Item { width: parent.width - abandon.width - finish.width - 236; height: 1 }
        AppButton {
            id: abandon
            text: qsTr("Abandon")
            variant: "danger"
            onClicked: abandonDialog.open()
        }
        AppButton {
            id: finish
            text: qsTr("Finish round")
            variant: "primary"
            onClicked: finishDialog.open()
        }
    }

    ConfirmDialog {
        id: abandonDialog
        title: qsTr("Abandon round?")
        bodyText: qsTr("The partial scorecard stays in history as abandoned.")
        onConfirmed: app.abandonRound()
    }
    ConfirmDialog {
        id: finishDialog
        title: qsTr("Finish round?")
        bodyText: qsTr("The scorecard will be saved to round history.")
        onConfirmed: app.finishRound()
    }
}
