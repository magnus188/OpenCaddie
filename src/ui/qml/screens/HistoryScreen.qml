import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    id: root
    anchors.fill: parent

    Component.onCompleted: app.refreshHistory("")

    function signed(value) {
        var number = Number(value)
        return number === 0 ? "E" : (number > 0 ? "+" : "") + number
    }

    function weatherLabel(value) {
        if (value === "partly_cloudy")
            return qsTr("Partly cloudy")
        return value
    }

    function statusLabel(value) {
        if (value === "completed")
            return qsTr("Completed")
        if (value === "in_progress")
            return qsTr("In progress")
        if (value === "abandoned")
            return qsTr("Abandoned")
        return value
    }

    function hasValue(value) {
        return value !== undefined && value !== null &&
               String(value).length > 0
    }

    function weatherSummary(round) {
        var parts = []
        if (hasValue(round.weatherCondition))
            parts.push(weatherLabel(round.weatherCondition))
        if (hasValue(round.weatherTemperatureC))
            parts.push(Math.round(Number(round.weatherTemperatureC)) + "°C")
        if (hasValue(round.weatherWindMps))
            parts.push(Number(round.weatherWindMps).toFixed(1) + " m/s")
        return parts.length > 0 ? parts.join(" · ")
                                : qsTr("Weather not recorded")
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Round history")
    }

    AppTextField {
        id: search
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        height: 48
        placeholderText: qsTr("Search course")
        onTextChanged: app.refreshHistory(text)
    }

    ListView {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: search.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 16
        anchors.topMargin: 8
        anchors.bottomMargin: KeyboardController.active
                              ? KeyboardController.keyboardHeight + 8 : 16
        model: app.history
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollIndicator.vertical: ScrollIndicator { }
        spacing: 0

        delegate: Rectangle {
            required property var modelData
            required property int index
            width: ListView.view.width
            height: 88
            color: historyTap.pressed
                   ? Theme.controlPressed : "transparent"

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.divider
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 5
                radius: 3
                color: modelData.status === "completed"
                       ? Theme.fairway : Theme.amber
            }

            Column {
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.verticalCenter: parent.verticalCenter
                width: 270
                spacing: 4
                Text {
                    width: parent.width
                    text: modelData.courseName
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(17)
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width
                    text: String(modelData.startedAt).slice(0, 10) + " · " +
                          qsTr("%1 holes").arg(modelData.holeCount) + " · " +
                          root.statusLabel(modelData.status)
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(11)
                }
                Text {
                    width: parent.width
                    text: root.weatherSummary(modelData)
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(10)
                    elide: Text.ElideRight
                }
            }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 320
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Repeater {
                    model: [
                        { label: qsTr("Eagles"), value: modelData.eagles,
                          color: Theme.amber },
                        { label: qsTr("Birdies"), value: modelData.birdies,
                          color: Theme.fairway },
                        { label: qsTr("Pars"), value: modelData.pars,
                          color: Theme.water }
                    ]
                    Item {
                        required property var modelData
                        width: 76
                        height: 50
                        Column {
                            anchors.centerIn: parent
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.value
                                color: modelData.color
                                font.family: "Inter"
                                font.weight: Font.Bold
                                font.pixelSize: Theme.px(18)
                            }
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.label
                                color: Theme.textMuted
                                font.family: "Inter"
                                font.pixelSize: Theme.px(9)
                            }
                        }
                    }
                }
            }

            Column {
                anchors.right: parent.right
                anchors.rightMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0
                property bool stablefordReady:
                    modelData.scoringMode === "stableford" &&
                    Boolean(modelData.stablefordAvailable)
                Text {
                    anchors.right: parent.right
                    text: parent.stablefordReady
                          ? qsTr("%1 pts").arg(modelData.stablefordPoints)
                          : modelData.toParAvailable
                            ? root.signed(modelData.toPar) : "—"
                    color: !parent.stablefordReady &&
                           !modelData.toParAvailable
                           ? Theme.textMuted
                           : parent.stablefordReady ||
                             Number(modelData.toPar) <= 0
                             ? Theme.fairway : Theme.amber
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(28)
                }
                Text {
                    anchors.right: parent.right
                    text: parent.stablefordReady
                          ? qsTr("Gross %1").arg(modelData.gross)
                          : root.statusLabel(modelData.status)
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(10)
                }
            }

            TapHandler {
                id: historyTap
                onTapped: {
                    KeyboardController.close()
                    app.selectHistoryRound(modelData.id)
                    app.navigateTo("RoundDetailScreen")
                }
            }
        }

        Text {
            anchors.centerIn: parent
            visible: app.history.length === 0
            text: qsTr("No saved rounds yet")
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(14)
        }
    }

}
