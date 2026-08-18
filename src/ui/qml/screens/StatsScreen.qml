import QtQuick
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    Component.onCompleted: app.refreshStatistics("")

    function signed(value) {
        var rounded = Math.round(Number(value) * 10) / 10
        return rounded === 0 ? "E" : (rounded > 0 ? "+" : "") + rounded
    }

    function maximumTrend() {
        var values = app.statistics.trend || []
        var maximum = 1
        for (var index = 0; index < values.length; ++index)
            maximum = Math.max(maximum,
                               Math.abs(Number(values[index].normalizedToPar)))
        return maximum
    }

    function fairwayPercentage(key) {
        var values = app.statistics.fairwayDistribution || []
        for (var index = 0; index < values.length; ++index) {
            if (values[index].key === key)
                return Math.round(Number(values[index].percentage))
        }
        return 0
    }

    function percentage(value, recorded) {
        return Number(recorded || 0) > 0
               ? Math.round(Number(value || 0)) + "%" : "—"
    }

    function average(value, recorded) {
        return Number(recorded || 0) > 0
               ? Number(value || 0).toFixed(1) : "—"
    }

    function shotCount(key) {
        var values = app.statistics.shotTypeDistribution || []
        for (var index = 0; index < values.length; ++index) {
            if (values[index].key === key)
                return Number(values[index].count || 0)
        }
        return 0
    }

    function outcomeLabel(key) {
        switch (key) {
        case "albatross": return qsTr("Albatross")
        case "eagle": return qsTr("Eagle")
        case "birdie": return qsTr("Birdie")
        case "par": return qsTr("Par")
        case "bogey": return qsTr("Bogey")
        case "double_or_worse": return qsTr("Double+")
        default: return key
        }
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Statistics")
    }

    AppSelectSheet {
        id: courseFilter
        anchors.left: parent.left
        anchors.top: header.bottom
        anchors.leftMargin: 16
        width: 250
        height: 48
        textRole: "courseName"
        valueRole: "courseSlug"
        model: {
            var courses = [{
                courseName: qsTr("All courses"),
                courseSlug: ""
            }]
            var available = app.statistics.courses || []
            for (var index = 0; index < available.length; ++index)
                courses.push(available[index])
            return courses
        }
        onActivated: app.refreshStatistics(currentValue)
    }

    Text {
        anchors.left: courseFilter.right
        anchors.leftMargin: 14
        anchors.verticalCenter: courseFilter.verticalCenter
        text: qsTr("%1 completed rounds · %2 holes")
              .arg(app.statistics.rounds || 0)
              .arg(app.statistics.holes || 0)
        color: Theme.textMuted
        font.family: "Inter"
        font.pixelSize: Theme.px(12)
    }

    Row {
        id: metrics
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: courseFilter.bottom
        anchors.margins: 16
        anchors.topMargin: 8
        height: 76
        spacing: 10

        MetricCard {
            width: (metrics.width - 30) / 4
            height: parent.height
            label: qsTr("Average to par")
            value: Number(app.statistics.toParRounds || 0) > 0
                   ? root.signed(app.statistics.averageToPar || 0) : "—"
            accent: Theme.amber
            prominent: true
        }
        MetricCard {
            width: (metrics.width - 30) / 4
            height: parent.height
            label: qsTr("Best round")
            value: Number(app.statistics.toParRounds || 0) > 0
                   ? root.signed(app.statistics.bestToPar || 0) : "—"
            accent: Theme.fairway
        }
        MetricCard {
            width: (metrics.width - 30) / 4
            height: parent.height
            label: qsTr("Consistency")
            value: Number(app.statistics.toParRounds || 0) > 0
                   ? "±" + Number(app.statistics.consistency || 0).toFixed(1)
                   : "—"
        }
        MetricCard {
            width: (metrics.width - 30) / 4
            height: parent.height
            label: qsTr("Longest drive")
            value: app.statistics.longestDriveRecorded
                   ? app.distanceText(app.statistics.longestDriveMetres) : "—"
            accent: Theme.water
        }
    }

    Row {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: metrics.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 16
        anchors.topMargin: 0
        spacing: 12

        SectionCard {
            width: 448
            height: parent.height
            title: qsTr("Recent form")

            Item {
                anchors.fill: parent

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 30
                    height: 1
                    color: Theme.border
                }

                Row {
                    anchors.fill: parent
                    anchors.topMargin: 4
                    anchors.bottomMargin: 2
                    spacing: 5

                    Repeater {
                        model: app.statistics.trend || []
                        delegate: Item {
                            required property var modelData
                            required property int index
                            width: 45
                            height: parent.height

                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: dateLabel.top
                                anchors.bottomMargin: 5
                                width: 22
                                height: Math.max(
                                    4,
                                    Math.abs(Number(modelData.normalizedToPar)) /
                                    root.maximumTrend() * 92)
                                radius: 4
                                color: Number(modelData.normalizedToPar) <= 0
                                       ? Theme.fairway : Theme.amber
                                opacity: 0.9
                            }
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: dateLabel.top
                                anchors.bottomMargin: Math.max(
                                    12,
                                    Math.abs(Number(modelData.normalizedToPar)) /
                                    root.maximumTrend() * 92 + 8)
                                text: root.signed(modelData.normalizedToPar)
                                color: Theme.text
                                font.family: "Inter"
                                font.weight: Font.DemiBold
                                font.pixelSize: Theme.px(10)
                            }
                            Text {
                                id: dateLabel
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: parent.bottom
                                width: parent.width
                                text: String(modelData.startedAt).slice(5, 10)
                                color: Theme.textMuted
                                font.family: "Inter"
                                font.pixelSize: Theme.px(9)
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 9
                    visible: (app.statistics.trend || []).length === 0

                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 48
                        height: 48
                        radius: 24
                        color: "transparent"
                        border.width: 1
                        border.color: Theme.textMuted
                        Row {
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: 4
                            spacing: 3
                            Repeater {
                                model: [10, 17, 25]
                                Rectangle {
                                    required property int modelData
                                    width: 5
                                    height: modelData
                                    color: Theme.textMuted
                                }
                            }
                        }
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Finish a round to start your trend")
                        color: Theme.textMuted
                        font.family: "Inter"
                        font.pixelSize: Theme.px(13)
                    }
                }
            }
        }

        SectionCard {
            width: parent.width - 460
            height: parent.height
            title: qsTr("Scoring mix")

            Column {
                anchors.fill: parent
                spacing: 1

                Repeater {
                    model: app.statistics.scoreDistribution || []
                    delegate: Row {
                        required property var modelData
                        width: parent.width
                        height: 19
                        spacing: 7

                        Text {
                            width: 90
                            anchors.verticalCenter: parent.verticalCenter
                            text: root.outcomeLabel(modelData.key)
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(10)
                        }
                        Rectangle {
                            width: 105
                            height: 8
                            radius: 4
                            anchors.verticalCenter: parent.verticalCenter
                            color: Theme.surfaceRaised
                            Rectangle {
                                width: parent.width *
                                       Math.min(1, Number(modelData.percentage) / 100)
                                height: parent.height
                                radius: parent.radius
                                color: modelData.key === "birdie" ||
                                       modelData.key === "eagle"
                                       ? Theme.fairway
                                       : modelData.key === "par"
                                         ? Theme.water : Theme.amber
                            }
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.count
                            color: Theme.text
                            font.family: "Inter"
                            font.weight: Font.DemiBold
                            font.pixelSize: Theme.px(11)
                        }
                    }
                }

                Text {
                    width: parent.width
                    text: qsTr("Fairways %1 · GIR %2 · Putts %3")
                          .arg(root.percentage(
                              app.statistics.fairwayPct,
                              app.statistics.fairwaysRecorded))
                          .arg(root.percentage(
                              app.statistics.girPct,
                              app.statistics.greensRecorded))
                          .arg(root.average(
                              app.statistics.averagePutts,
                              app.statistics.puttsRecorded))
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(10)
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width
                    text: qsTr("L %1% · C %2% · R %3% · M %4%")
                          .arg(root.fairwayPercentage("left"))
                          .arg(root.fairwayPercentage("centre"))
                          .arg(root.fairwayPercentage("right"))
                          .arg(root.fairwayPercentage("missed"))
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(9)
                    elide: Text.ElideRight
                    visible: Number(app.statistics.fairwaysRecorded || 0) > 0
                }
                Text {
                    width: parent.width
                    text: qsTr("%1 tracked · %2 scored")
                          .arg(app.statistics.trackedStrokes || 0)
                          .arg(app.statistics.scoredStrokes || 0)
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.DemiBold
                    font.pixelSize: Theme.px(10)
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width
                    text: qsTr("Drive %1 · Approach %2 · Chip %3")
                          .arg(root.shotCount("drive"))
                          .arg(root.shotCount("approach"))
                          .arg(root.shotCount("chip"))
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(9)
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width
                    text: qsTr("Putt %1 · Unknown %2")
                          .arg(root.shotCount("putt"))
                          .arg(root.shotCount("unknown"))
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(9)
                    elide: Text.ElideRight
                }
            }
        }
    }
}
