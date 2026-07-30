import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    property string selectedId: ""
    property int selectedIndex: -1

    function selectClub(index, club) {
        selectedIndex = index
        selectedId = club.id
        clubName.text = club.name
        clubCarry.text = Math.round(club.carry).toString()
        clubEnabled.checked = club.enabled
    }

    function moveSelected(delta) {
        if (selectedIndex < 0)
            return
        var target = selectedIndex + delta
        if (target < 0 || target >= app.clubs.length)
            return
        var ids = []
        for (var i = 0; i < app.clubs.length; ++i)
            ids.push(app.clubs[i].id)
        var moved = ids.splice(selectedIndex, 1)[0]
        ids.splice(target, 0, moved)
        app.reorderClubs(ids)
        selectedIndex = target
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        title: qsTr("My bag")
        subtitle: qsTr("Carry distances drive on-course recommendations")
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
            width: 390
            height: parent.height
            title: qsTr("Enabled clubs")
            subtitle: qsTr("Stored canonically in metres")

            ListView {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: orderButtons.top
                anchors.bottomMargin: 8
                model: app.clubs
                clip: true
                spacing: 5

                delegate: Rectangle {
                    required property var modelData
                    required property int index
                    width: ListView.view.width
                    height: 52
                    radius: Theme.radius
                    color: root.selectedId === modelData.id
                           ? Qt.rgba(0.18, 0.80, 0.39, 0.14)
                           : Theme.surfaceRaised
                    border.width: 1
                    border.color: root.selectedId === modelData.id
                                  ? Theme.fairway : Theme.border
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.name
                        color: modelData.enabled ? Theme.text : Theme.textMuted
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(15)
                    }
                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: Math.round(modelData.carry) + " " + modelData.unit
                        color: Theme.fairway
                        font.family: "Inter"
                        font.weight: Font.Bold
                        font.pixelSize: Theme.px(15)
                    }
                    TapHandler {
                        onTapped: root.selectClub(index, modelData)
                    }
                }
            }

            Row {
                id: orderButtons
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                spacing: 8
                AppButton {
                    text: qsTr("Move up")
                    compact: true
                    enabled: root.selectedIndex > 0
                    onClicked: root.moveSelected(-1)
                }
                AppButton {
                    text: qsTr("Move down")
                    compact: true
                    enabled: root.selectedIndex >= 0 &&
                             root.selectedIndex < app.clubs.length - 1
                    onClicked: root.moveSelected(1)
                }
            }
        }

        SectionCard {
            width: parent.width - 404
            height: parent.height
            title: root.selectedId.length > 0 ? qsTr("Edit club") : qsTr("Add club")

            Column {
                anchors.fill: parent
                spacing: 9
                Text {
                    text: qsTr("Club name")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(12)
                }
                AppTextField {
                    id: clubName
                    width: parent.width
                    placeholderText: qsTr("Example: 7 iron")
                }
                Text {
                    text: app.metric ? qsTr("Carry (metres)") : qsTr("Carry (yards)")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(12)
                }
                AppTextField {
                    id: clubCarry
                    width: parent.width
                    placeholderText: "145"
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                }
                Row {
                    spacing: 10
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Use for recommendations")
                        color: Theme.text
                        font.family: "Inter"
                        font.pixelSize: Theme.px(13)
                    }
                    Switch {
                        id: clubEnabled
                        checked: true
                    }
                }
                Row {
                    spacing: 8
                    AppButton {
                        text: root.selectedId.length > 0 ? qsTr("Save") : qsTr("Add")
                        variant: "primary"
                        onClicked: {
                            if (root.selectedId.length > 0)
                                app.updateClub(root.selectedId, clubName.text,
                                               parseFloat(clubCarry.text) || 0,
                                               clubEnabled.checked)
                            else
                                app.addClub(clubName.text,
                                            parseFloat(clubCarry.text) || 0)
                            root.selectedId = ""
                            root.selectedIndex = -1
                            clubName.text = ""
                            clubCarry.text = ""
                            clubEnabled.checked = true
                        }
                    }
                    AppButton {
                        text: qsTr("New")
                        onClicked: {
                            root.selectedId = ""
                            root.selectedIndex = -1
                            clubName.text = ""
                            clubCarry.text = ""
                            clubEnabled.checked = true
                        }
                    }
                    AppButton {
                        text: qsTr("Remove")
                        variant: "danger"
                        enabled: root.selectedId.length > 0
                        onClicked: removeDialog.open()
                    }
                }
            }
        }
    }

    ConfirmDialog {
        id: removeDialog
        title: qsTr("Remove club?")
        bodyText: qsTr("The club will no longer be available for recommendations.")
        onConfirmed: {
            app.removeClub(root.selectedId)
            root.selectedId = ""
            root.selectedIndex = -1
        }
    }
}

