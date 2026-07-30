import QtQuick
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    property string selectedSlug: app.courses.length > 0 ? app.courses[0].slug : ""
    property var selectedCourse: {
        for (var i = 0; i < app.courses.length; ++i) {
            if (app.courses[i].slug === selectedSlug)
                return app.courses[i]
        }
        return ({})
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        title: qsTr("Courses")
        subtitle: qsTr("Downloaded courses work without internet")
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
            width: 378
            height: parent.height
            title: qsTr("Offline library")
            subtitle: qsTr("%1 courses").arg(app.courses.length)

            ListView {
                id: localList
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: localActions.top
                anchors.bottomMargin: 8
                spacing: 8
                clip: true
                model: app.courses

                delegate: Rectangle {
                    required property var modelData
                    width: ListView.view.width
                    height: 72
                    radius: Theme.radius
                    color: root.selectedSlug === modelData.slug
                           ? Qt.rgba(0.18, 0.80, 0.39, 0.14)
                           : Theme.surfaceRaised
                    border.width: root.selectedSlug === modelData.slug ? 2 : 1
                    border.color: root.selectedSlug === modelData.slug
                                  ? Theme.fairway : Theme.border

                    Column {
                        anchors.left: parent.left
                        anchors.right: quality.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 12
                        anchors.rightMargin: 10
                        spacing: 2
                        Text {
                            width: parent.width
                            text: modelData.name
                            color: Theme.text
                            font.family: "Inter"
                            font.weight: Font.DemiBold
                            font.pixelSize: Theme.px(15)
                            elide: Text.ElideRight
                        }
                        Text {
                            width: parent.width
                            text: qsTr("Offline · %1% quality · © OSM (ODbL)")
                                      .arg(modelData.qualityScore)
                            color: Theme.textMuted
                            font.family: "Inter"
                            font.pixelSize: Theme.px(11)
                            elide: Text.ElideRight
                        }
                    }
                    Rectangle {
                        id: quality
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        width: 50
                        height: 30
                        radius: 15
                        color: Theme.surface
                        Text {
                            anchors.centerIn: parent
                            text: modelData.qualityScore + "%"
                            color: modelData.qualityScore >= 75
                                   ? Theme.fairway : Theme.amber
                            font.family: "Inter"
                            font.weight: Font.DemiBold
                            font.pixelSize: Theme.px(12)
                        }
                    }
                    TapHandler {
                        onTapped: root.selectedSlug = modelData.slug
                    }
                }
            }

            Row {
                id: localActions
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                spacing: 8
                AppButton {
                    id: removeButton
                    text: qsTr("Remove")
                    variant: "danger"
                    compact: true
                    enabled: root.selectedSlug.length > 0 &&
                             root.selectedSlug !== "opencaddie-demo"
                    onClicked: removeDialog.open()
                }
                Item {
                    width: Math.max(0, parent.width - removeButton.width -
                                    setupButton.width - 8)
                    height: 1
                }
                AppButton {
                    id: setupButton
                    text: qsTr("Set up round")
                    variant: "primary"
                    enabled: root.selectedSlug.length > 0
                    onClicked: app.prepareRound(root.selectedSlug)
                }
            }
        }

        SectionCard {
            width: parent.width - 392
            height: parent.height
            title: qsTr("Find a course")
            subtitle: network.internetReachable
                      ? qsTr("OpenGolfMap service")
                      : qsTr("Connect to Wi-Fi to search")

            Row {
                id: searchRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: 8
                AppTextField {
                    id: searchField
                    width: parent.width - searchButton.width - 8
                    placeholderText: qsTr("Course name or place")
                    onAccepted: app.searchCourses(text)
                }
                AppButton {
                    id: searchButton
                    text: app.searching ? qsTr("Searching…") : qsTr("Search")
                    compact: true
                    enabled: !app.searching && searchField.text.length >= 2
                    onClicked: app.searchCourses(searchField.text)
                }
            }

            ListView {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: searchRow.bottom
                anchors.bottom: wifiButton.top
                anchors.topMargin: 10
                anchors.bottomMargin: 8
                clip: true
                spacing: 7
                model: app.searchResults

                delegate: Rectangle {
                    required property var modelData
                    width: ListView.view.width
                    height: 66
                    radius: Theme.radius
                    color: Theme.surfaceRaised
                    border.width: 1
                    border.color: Theme.border
                    Text {
                        anchors.left: parent.left
                        anchors.right: download.left
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: 10
                        text: modelData.displayName
                        color: Theme.text
                        font.family: "Inter"
                        font.pixelSize: Theme.px(13)
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                    AppButton {
                        id: download
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Download")
                        compact: true
                        enabled: !app.downloading
                        onClicked: app.downloadCourse(modelData)
                    }
                }
            }

            AppButton {
                id: wifiButton
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                text: qsTr("Wi-Fi settings")
                compact: true
                onClicked: app.screen = "WifiScreen"
            }
            Text {
                anchors.left: wifiButton.right
                anchors.leftMargin: 12
                anchors.verticalCenter: wifiButton.verticalCenter
                text: app.downloading
                      ? qsTr("Downloading %1%").arg(Math.round(app.downloadProgress * 100))
                      : ""
                color: Theme.fairway
                font.family: "Inter"
                font.pixelSize: Theme.px(12)
            }
        }
    }

    ConfirmDialog {
        id: removeDialog
        title: qsTr("Remove offline course?")
        bodyText: qsTr("The course can be downloaded again later. Round history is kept.")
        onConfirmed: {
            app.removeCourse(root.selectedCourse.slug,
                             root.selectedCourse.version)
            root.selectedSlug = app.courses.length > 0
                                ? app.courses[0].slug : ""
        }
    }
}
