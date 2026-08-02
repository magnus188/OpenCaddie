import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    anchors.fill: parent

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Privacy and diagnostics")
        onBack: app.screen = "SettingsScreen"
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: header.bottom
        width: 560
        spacing: 10

        Rectangle {
            width: parent.width
            height: 60
            color: "transparent"
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: Theme.divider
            }
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Privacy-safe diagnostic logging")
                color: Theme.text
                font.family: "Inter"
                font.pixelSize: Theme.px(14)
            }
            Switch {
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                checked: app.diagnosticLogging
                onToggled: app.diagnosticLogging = checked
            }
        }

        SectionCard {
            width: parent.width
            height: 92
            title: qsTr("GPS diagnostics")
            Text {
                anchors.fill: parent
                text: qsTr("Status: %1 · Accuracy ±%2 m")
                      .arg(app.gpsStatus).arg(Math.round(app.gpsAccuracy))
                color: Theme.textMuted
                font.family: "Inter"
                font.pixelSize: Theme.px(12)
                verticalAlignment: Text.AlignVCenter
            }
        }

        Text {
            width: parent.width
            text: qsTr("Vendor passwords are never stored. Round history, clubs, and courses remain local unless you explicitly export or sync them.")
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(11)
            wrapMode: Text.WordWrap
        }

        AppButton {
            width: parent.width
            text: qsTr("Reset settings")
            variant: "danger"
            onClicked: resetDialog.open()
        }
    }

    ConfirmDialog {
        id: resetDialog
        title: qsTr("Reset settings?")
        bodyText: qsTr("Round history, clubs, and downloaded courses are kept.")
        onConfirmed: app.resetSettings()
    }
}
