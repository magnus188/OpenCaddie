import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    id: root
    anchors.fill: parent

    function providerSummary(provider) {
        switch (provider) {
        case "garmin_golf":
            return qsTr("Read access needs Garmin approval and a licence. Golf score write-back is not documented.")
        case "garmin_connect":
            return qsTr("Approved business access can provide activity files. Golf-specific completeness is not guaranteed.")
        case "trackman":
            return qsTr("A user-initiated TrackMan CSV importer is planned. Cloud automation needs a partner agreement.")
        case "toptracer":
            return qsTr("The game-data API is rich. Production onboarding must be confirmed with Toptracer.")
        case "golfbox_no":
            return qsTr("Handicap, score submission, and booking stay disabled until NGF/GolfBox approves access.")
        default:
            return ""
        }
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Data sources")
    }

    ListView {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 16
        anchors.topMargin: 0
        model: app.integrations
        spacing: 0
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollIndicator.vertical: ScrollIndicator { }

        delegate: Rectangle {
            required property var modelData
            width: ListView.view.width
            height: 76
            color: "transparent"

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.divider
            }

            Column {
                anchors.left: parent.left
                anchors.right: status.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 3
                Text {
                    text: modelData.name
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(15)
                }
                Text {
                    width: parent.width
                    text: root.providerSummary(modelData.id)
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(10)
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                }
            }
            Rectangle {
                id: status
                anchors.right: parent.right
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                width: 126
                height: 34
                radius: 17
                color: modelData.availability === "planned_file_import"
                       ? Qt.rgba(0.17, 0.65, 0.84, 0.14)
                       : Qt.rgba(0.83, 0.54, 0.21, 0.14)
                Text {
                    anchors.centerIn: parent
                    text: modelData.availability === "planned_file_import"
                          ? qsTr("Planned import")
                          : qsTr("Partner required")
                    color: modelData.availability === "planned_file_import"
                           ? Theme.water : Theme.amber
                    font.family: "Inter"
                    font.weight: Font.DemiBold
                    font.pixelSize: Theme.px(10)
                }
            }
        }
    }
}
