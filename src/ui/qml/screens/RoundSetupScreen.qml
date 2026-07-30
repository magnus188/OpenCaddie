import QtQuick
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    property int holes: 18
    property bool stableford: false

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        title: qsTr("Set up round")
        subtitle: app.selectedCourseName
        onBack: app.screen = "CourseLibraryScreen"
    }

    Row {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: footer.top
        anchors.margins: 16
        anchors.topMargin: 4
        anchors.bottomMargin: 12
        spacing: 14

        SectionCard {
            width: (parent.width - 14) / 2
            height: parent.height
            title: qsTr("Round format")
            Column {
                anchors.fill: parent
                spacing: 12
                Text {
                    text: qsTr("Holes")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(12)
                }
                Row {
                    spacing: 8
                    AppButton {
                        text: qsTr("9 holes")
                        variant: root.holes === 9 ? "primary" : "secondary"
                        onClicked: root.holes = 9
                    }
                    AppButton {
                        text: qsTr("18 holes")
                        variant: root.holes === 18 ? "primary" : "secondary"
                        onClicked: root.holes = 18
                    }
                }
                Text {
                    text: qsTr("Scoring")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(12)
                }
                Row {
                    spacing: 8
                    AppButton {
                        text: qsTr("Stroke play")
                        variant: !root.stableford ? "primary" : "secondary"
                        onClicked: root.stableford = false
                    }
                    AppButton {
                        text: qsTr("Stableford")
                        variant: root.stableford ? "primary" : "secondary"
                        onClicked: root.stableford = true
                    }
                }
            }
        }

        SectionCard {
            width: (parent.width - 14) / 2
            height: parent.height
            title: qsTr("Player and tee")
            Column {
                anchors.fill: parent
                spacing: 10
                Text {
                    text: qsTr("Course handicap")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(12)
                }
                AppTextField {
                    id: handicap
                    width: parent.width
                    text: "0"
                    inputMethodHints: Qt.ImhDigitsOnly
                }
                Text {
                    text: qsTr("Tee")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(12)
                }
                AppTextField {
                    id: tee
                    width: parent.width
                    text: qsTr("Yellow")
                }
                Text {
                    width: parent.width
                    text: qsTr("This round and course remain available without internet.")
                    color: Theme.fairway
                    font.family: "Inter"
                    font.pixelSize: Theme.px(11)
                    wrapMode: Text.WordWrap
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
        height: 52
        spacing: 10
        AppButton {
            text: qsTr("Cancel")
            onClicked: app.screen = "CourseLibraryScreen"
        }
        Item { width: parent.width - startButton.width - 122; height: 1 }
        AppButton {
            id: startButton
            text: qsTr("Start round")
            variant: "primary"
            onClicked: app.startRound(app.selectedCourseSlug, root.holes,
                                      root.stableford,
                                      parseInt(handicap.text) || 0, tee.text)
        }
    }
}
