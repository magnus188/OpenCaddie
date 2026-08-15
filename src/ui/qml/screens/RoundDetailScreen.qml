import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    property var summary: app.roundDetail.summary || ({})
    property var weather: app.roundDetail.weather || ({})

    function signed(value) {
        var number = Number(value)
        return number === 0 ? "E" : (number > 0 ? "+" : "") + number
    }

    function weatherLabel(value) {
        if (value === "partly_cloudy")
            return qsTr("Partly cloudy")
        if (value === "sunny")
            return qsTr("Sunny")
        if (value === "cloudy")
            return qsTr("Cloudy")
        if (value === "rain")
            return qsTr("Rain")
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

    function weatherMeasurements() {
        var parts = []
        if (hasValue(root.weather.temperatureC))
            parts.push(Math.round(Number(root.weather.temperatureC)) + "°C")
        if (hasValue(root.weather.windMps))
            parts.push(Number(root.weather.windMps).toFixed(1) + " m/s")
        return parts.join(" · ")
    }

    function percentage(value, recorded) {
        return Number(recorded || 0) > 0
               ? Math.round(Number(value || 0)) + "%" : "—"
    }

    function average(value, recorded) {
        return Number(recorded || 0) > 0
               ? Number(value || 0).toFixed(1) : "—"
    }

    function optionalValue(value) {
        return value === undefined || value === null ? "—" : value
    }

    function fairwayLabel(value) {
        if (value === "left") return qsTr("Left")
        if (value === "centre") return qsTr("Centre")
        if (value === "right") return qsTr("Right")
        if (value === "missed") return qsTr("Missed")
        return "—"
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: app.roundDetail.courseName || qsTr("Round details")
        subtitle: String(app.roundDetail.startedAt || "").slice(0, 10) +
                  (app.roundDetail.tee ? " · " + app.roundDetail.tee : "") +
                  (app.roundDetail.status
                   ? " · " + root.statusLabel(app.roundDetail.status) : "")
    }

    Row {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 16
        anchors.topMargin: 2
        spacing: 12

        SectionCard {
            width: 286
            height: parent.height
            title: qsTr("Round overview")

            Column {
                anchors.fill: parent
                spacing: 9

                Row {
                    width: parent.width
                    height: 62
                    spacing: 8
                    MetricCard {
                        width: 122
                        height: parent.height
                        label: app.roundDetail.scoringMode === "stableford"
                               ? qsTr("Points") : qsTr("To par")
                        value: app.roundDetail.scoringMode === "stableford"
                               ? root.summary.stablefordAvailable
                                 ? String(root.summary.stablefordPoints) : "—"
                               : root.summary.toParAvailable
                                 ? root.signed(root.summary.toPar) : "—"
                        accent: (app.roundDetail.scoringMode === "stableford"
                                 ? root.summary.stablefordAvailable
                                 : root.summary.toParAvailable)
                                ? Number(root.summary.toPar) <= 0
                                  ? Theme.fairway : Theme.amber
                                : Theme.textMuted
                        prominent: true
                    }
                    MetricCard {
                        width: 122
                        height: parent.height
                        label: qsTr("Gross")
                        value: Number(root.summary.scoredHoles || 0) > 0
                               ? root.summary.gross : "—"
                    }
                }

                Row {
                    width: parent.width
                    spacing: 5
                    Repeater {
                        model: [
                            { label: qsTr("Eagle"),
                              value: root.summary.eagles || 0,
                              color: Theme.amber },
                            { label: qsTr("Birdie"),
                              value: root.summary.birdies || 0,
                              color: Theme.fairway },
                            { label: qsTr("Par"),
                              value: root.summary.pars || 0,
                              color: Theme.water }
                        ]
                        Item {
                            required property var modelData
                            width: 80
                            height: 43
                            Text {
                                anchors.centerIn: parent
                                text: modelData.value + " " + modelData.label
                                color: modelData.color
                                font.family: "Inter"
                                font.weight: Font.DemiBold
                                font.pixelSize: Theme.px(10)
                            }
                        }
                    }
                }

                Rectangle {
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
                    Column {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2
                        Text {
                            text: root.hasValue(root.weather.condition)
                                  ? root.weatherLabel(root.weather.condition)
                                  : root.weather.recordedAt
                                    ? qsTr("Weather recorded")
                                    : qsTr("Weather not recorded")
                            color: Theme.text
                            font.family: "Inter"
                            font.weight: Font.DemiBold
                            font.pixelSize: Theme.px(11)
                        }
                        Text {
                            text: root.weather.recordedAt
                                  ? (root.weatherMeasurements() ||
                                     qsTr("Measurements unavailable")) +
                                    (root.weather.source
                                     ? " · " + qsTr("Source: %1")
                                                  .arg(root.weather.source)
                                     : "")
                                  : qsTr("Captured only when a source is available")
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(9)
                        }
                    }
                }

                Text {
                    width: parent.width
                    text: qsTr("Fairways %1 · GIR %2")
                          .arg(root.percentage(
                              root.summary.fairwayPct,
                              root.summary.fairwaysRecorded))
                          .arg(root.percentage(
                              root.summary.girPct,
                              root.summary.greensRecorded))
                    color: Theme.text
                    font.family: "Inter"
                    font.pixelSize: Theme.px(11)
                }
                Text {
                    width: parent.width
                    text: qsTr("Average putts %1 · Penalties %2")
                          .arg(root.average(
                              root.summary.averagePutts,
                              root.summary.puttsRecorded))
                          .arg(root.summary.penalties || 0)
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(10)
                }
                Text {
                    width: parent.width
                    text: qsTr("Longest drive: %1").arg(
                              root.summary.longestDriveMetres > 0
                              ? app.distanceText(
                                    root.summary.longestDriveMetres)
                              : qsTr("Not recorded"))
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(10)
                }
            }
        }

        SectionCard {
            width: parent.width - 298
            height: parent.height
            title: qsTr("Scorecard")

            ListView {
                id: scoreList
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: actions.top
                anchors.bottomMargin: 6
                model: app.roundDetail.scores || []
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                ScrollIndicator.vertical: ScrollIndicator { }
                spacing: 0

                header: Row {
                    width: ListView.view.width
                    height: 24
                    Repeater {
                        model: [
                            { text: qsTr("Hole"), width: 42 },
                            { text: qsTr("Par"), width: 36 },
                            { text: qsTr("SI"), width: 34 },
                            { text: qsTr("Score"), width: 50 },
                            { text: qsTr("Pts"), width: 38 },
                            { text: qsTr("Putts"), width: 46 },
                            { text: qsTr("Pen."), width: 38 },
                            { text: qsTr("Fairway"), width: 72 },
                            { text: qsTr("GIR"), width: 36 }
                        ]
                        Text {
                            required property var modelData
                            width: modelData.width
                            height: parent.height
                            text: modelData.text
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(10)
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                delegate: Rectangle {
                    required property var modelData
                    required property int index
                    width: ListView.view.width
                    height: 29
                    color: "transparent"
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
                                { text: modelData.hole, width: 42 },
                                { text: modelData.par, width: 36 },
                                { text: modelData.strokeIndex, width: 34 },
                                { text: modelData.strokes, width: 50 },
                                { text: app.roundDetail.scoringMode === "stableford"
                                        ? root.optionalValue(
                                            modelData.stablefordPoints)
                                        : "—", width: 38 },
                                { text: root.optionalValue(modelData.putts),
                                  width: 46 },
                                { text: modelData.penalties, width: 38 },
                                { text: root.fairwayLabel(modelData.fairway),
                                  width: 72 },
                                { text: root.hasValue(modelData.gir)
                                        ? modelData.gir ? qsTr("Yes")
                                                        : qsTr("No")
                                        : "—", width: 36 }
                            ]
                            Text {
                                required property var modelData
                                width: modelData.width
                                height: parent.height
                                text: modelData.text
                                color: Theme.text
                                font.family: "Inter"
                                font.pixelSize: Theme.px(11)
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
                    onClicked: app.exportRound(app.roundDetail.id, "json")
                }
                AppButton {
                    text: qsTr("CSV")
                    compact: true
                    onClicked: app.exportRound(app.roundDetail.id, "csv")
                }
                AppButton {
                    visible: app.roundDetail.status === "in_progress"
                    text: qsTr("Resume")
                    compact: true
                    variant: "primary"
                    onClicked: app.resumeRound()
                }
                Item {
                    width: Math.max(0, parent.width - 261)
                    height: 1
                }
                AppButton {
                    visible: app.roundDetail.status !== "in_progress"
                    text: qsTr("Delete")
                    compact: true
                    variant: "danger"
                    onClicked: deleteDialog.open()
                }
            }
        }
    }

    ConfirmDialog {
        id: deleteDialog
        title: qsTr("Delete round?")
        bodyText: qsTr("This permanently removes the local scorecard and statistics.")
        destructive: true
        confirmText: qsTr("Delete")
        onConfirmed: {
            if (app.deleteRound(app.roundDetail.id))
                app.goBack()
        }
    }
}
