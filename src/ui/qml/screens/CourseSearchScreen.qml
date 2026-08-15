import QtQuick
import QtQuick.Controls
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    property string pendingKey
    property var completed: ({})
    property int completionRevision: 0

    function keyFor(candidate) {
        return candidate.displayName || candidate.name || JSON.stringify(candidate)
    }
    function downloaded(candidate) {
        var ignored = completionRevision
        return completed[keyFor(candidate)] === true
    }

    Connections {
        target: app
        function onCoursesChanged() {
            if (root.pendingKey.length === 0) return
            var next = Object.assign({}, root.completed)
            next[root.pendingKey] = true
            root.completed = next
            root.pendingKey = ""
            root.completionRevision += 1
        }
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Find courses")
    }

    Row {
        id: searchRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        spacing: 8

        AppTextField {
            id: searchField
            width: parent.width - searchButton.width - parent.spacing
            placeholderText: qsTr("Course name or place")
            onAccepted: app.searchCourses(text)
        }
        AppButton {
            id: searchButton
            width: 112
            text: app.searching ? qsTr("Searching…") : qsTr("Search")
            variant: "primary"
            compact: true
            enabled: !app.searching && searchField.text.trim().length >= 2 &&
                     network.internetReachable
            onClicked: app.searchCourses(searchField.text)
        }
    }

    Row {
        id: serviceState
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchRow.bottom
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        height: 38
        spacing: 8

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: !network.internetReachable ? qsTr("Wi-Fi required")
                  : app.openGolfMapReachable ? qsTr("OpenGolfMap connected")
                  : qsTr("OpenGolfMap unavailable")
            color: network.internetReachable && app.openGolfMapReachable
                   ? Theme.fairway : Theme.amber
            font.family: "Inter"
            font.pixelSize: Theme.px(12)
        }
        AppButton {
            text: qsTr("Wi-Fi")
            compact: true
            variant: "accent"
            visible: !network.internetReachable
            onClicked: app.navigateTo("WifiScreen")
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: app.downloading
                  ? qsTr("Downloading %1%").arg(Math.round(app.downloadProgress * 100))
                  : ""
            color: Theme.fairway
            font.family: "Inter"
            font.pixelSize: Theme.px(12)
        }
    }

    ListView {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: serviceState.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        anchors.bottomMargin: 12
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollIndicator.vertical: ScrollIndicator { }
        model: app.searchResults

        delegate: Item {
            id: resultRow
            required property var modelData
            width: ListView.view.width
            height: 62
            Text {
                anchors.left: parent.left
                anchors.right: downloadButton.left
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: resultRow.modelData.displayName || resultRow.modelData.name
                color: Theme.text
                font.family: "Inter"
                font.weight: Font.Medium
                font.pixelSize: Theme.px(15)
                elide: Text.ElideRight
            }
            AppButton {
                id: downloadButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 124
                compact: true
                text: root.downloaded(resultRow.modelData)
                      ? qsTr("Downloaded") : qsTr("Download")
                variant: root.downloaded(resultRow.modelData) ? "accent" : "surface"
                enabled: !app.downloading && !root.downloaded(resultRow.modelData)
                onClicked: {
                    root.pendingKey = root.keyFor(resultRow.modelData)
                    app.downloadCourse(resultRow.modelData)
                }
            }
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.divider
            }
        }
    }
}
