import QtQuick
import QtQuick.Controls
import OpenCaddie

Popup {
    id: sheet

    signal clubSelected(string clubId)

    parent: Overlay.overlay
    x: 0
    y: parent ? parent.height - height : 0
    width: parent ? parent.width : 800
    height: Math.min(parent ? parent.height - Theme.statusHeight : 458, 318)
    modal: true
    focus: true
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    enter: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: Theme.motionSheet
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                property: "y"
                from: sheet.parent ? sheet.parent.height : 480
                to: sheet.parent ? sheet.parent.height - sheet.height : 0
                duration: Theme.motionSlow
                easing.type: Easing.OutCubic
            }
        }
    }

    exit: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: Theme.motionFast
                easing.type: Easing.InCubic
            }
            NumberAnimation {
                property: "y"
                from: sheet.parent ? sheet.parent.height - sheet.height : 0
                to: sheet.parent ? sheet.parent.height : 480
                duration: Theme.motionSheet
                easing.type: Easing.InCubic
            }
        }
    }

    background: Rectangle {
        color: Theme.surface
        radius: Theme.sheetRadius
        border.width: 1
        border.color: Theme.border
    }

    Item {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 20
        anchors.rightMargin: 14
        height: 62

        Column {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            Text {
                text: qsTr("Which club?")
                color: Theme.text
                font.family: "Inter"
                font.weight: Font.Bold
                font.pixelSize: Theme.px(21)
            }
            Text {
                text: app.shotGpsAvailable
                      ? qsTr("Choose a club · GPS will be saved")
                      : qsTr("Choose a club · No GPS, records anyway")
                color: app.shotGpsAvailable ? Theme.textMuted : Theme.amber
                font.family: "Inter"
                font.pixelSize: Theme.px(11)
            }
        }

        IconButton {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            transparent: true
            iconSource: "../../assets/icons/lucide/x.svg"
            iconColor: Theme.text
            accessibleName: qsTr("Close club picker")
            onClicked: sheet.close()
        }
    }

    ListView {
        id: clubList
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: footer.top
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        anchors.bottomMargin: 6
        orientation: ListView.Horizontal
        model: app.clubs
        spacing: 8
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: 2200
        maximumFlickVelocity: 3000
        ScrollIndicator.horizontal: ScrollIndicator { }

        delegate: Button {
            id: clubButton
            required property var modelData
            required property int index

            width: 112
            height: clubList.height - 4
            padding: 0
            Accessible.role: Accessible.Button
            Accessible.name: qsTr("Record with %1").arg(modelData.name)
            onClicked: {
                sheet.clubSelected(modelData.id)
                sheet.close()
            }

            contentItem: Item {
                ClubArtwork {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    width: 88
                    height: 102
                    clubType: clubButton.modelData.type
                    clubEnabled: clubButton.modelData.enabled
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 105
                    anchors.leftMargin: 7
                    anchors.rightMargin: 7
                    text: clubButton.modelData.name
                    color: clubButton.modelData.enabled ? Theme.text : Theme.textMuted
                    font.family: "Inter"
                    font.weight: Font.DemiBold
                    font.pixelSize: Theme.px(12)
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 126
                    text: clubButton.modelData.enabled
                          ? Math.round(clubButton.modelData.carry) + " " +
                            clubButton.modelData.unit
                          : qsTr("Off")
                    color: clubButton.modelData.enabled
                           ? Theme.textMuted : Theme.amber
                    font.family: "Inter"
                    font.pixelSize: Theme.px(10)
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }
            }

            background: Rectangle {
                radius: 12
                color: clubButton.down ? Theme.controlPressed
                                        : Theme.surfaceRaised
                border.width: clubButton.activeFocus ? 2 : 1
                border.color: clubButton.activeFocus
                              ? Theme.fairway : Theme.border

                Behavior on color {
                    ColorAnimation { duration: Theme.motionFast }
                }
            }
        }

        Text {
            anchors.centerIn: parent
            visible: app.clubs.length === 0
            text: qsTr("No clubs in your bag yet")
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(14)
        }
    }

    Item {
        id: footer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.bottomMargin: 10
        height: Theme.touch

        AppButton {
            anchors.left: parent.left
            height: Theme.touch
            text: qsTr("Record without club")
            variant: "surface"
            compact: true
            accessibleName: qsTr("Record stroke without choosing a club")
            onClicked: {
                sheet.clubSelected("")
                sheet.close()
            }
        }

        Text {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 330
            text: qsTr("Club choice is optional and can be skipped.")
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(10)
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideRight
        }
    }
}
