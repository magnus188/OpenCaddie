import QtQuick
import QtQuick.Controls
import OpenCaddie

Popup {
    id: sheet

    property bool adding: true
    property string editId: ""
    property string baselineName: ""
    property string baselineCarry: ""
    property string baselineType: "iron"
    property bool baselineEnabled: true
    property var typeOptions: [
        { "text": qsTr("Driver"), "value": "driver" },
        { "text": qsTr("Fairway wood"), "value": "wood" },
        { "text": qsTr("Hybrid"), "value": "hybrid" },
        { "text": qsTr("Iron"), "value": "iron" },
        { "text": qsTr("Wedge"), "value": "wedge" },
        { "text": qsTr("Putter"), "value": "putter" },
        { "text": qsTr("Other"), "value": "other" }
    ]
    readonly property bool draftValid:
        clubName.text.trim().length > 0 &&
        isFinite(parseFloat(clubCarry.text)) && parseFloat(clubCarry.text) > 0
    readonly property bool draftDirty:
        clubName.text !== baselineName || clubCarry.text !== baselineCarry ||
        String(clubType.currentValue) !== baselineType ||
        recommendationSwitch.checked !== baselineEnabled

    signal saved(bool added, string clubId)

    function indexForType(type) {
        for (var i = 0; i < typeOptions.length; ++i) {
            if (typeOptions[i].value === type)
                return i
        }
        return typeOptions.length - 1
    }

    function setBaseline() {
        baselineName = clubName.text
        baselineCarry = clubCarry.text
        baselineType = String(clubType.currentValue)
        baselineEnabled = recommendationSwitch.checked
    }

    function openForAdd() {
        adding = true
        editId = ""
        clubName.text = ""
        clubCarry.text = app.metric ? "145" : "160"
        clubType.currentIndex = indexForType("iron")
        recommendationSwitch.checked = true
        setBaseline()
        open()
    }

    function openForEdit(club) {
        adding = false
        editId = club.id
        clubName.text = club.name
        clubCarry.text = Math.round(club.carry).toString()
        clubType.currentIndex = indexForType(club.type)
        recommendationSwitch.checked = club.enabled
        setBaseline()
        open()
    }

    function requestClose() {
        KeyboardController.close()
        if (draftDirty)
            discardDialog.open()
        else
            close()
    }

    function commit() {
        if (!draftValid)
            return
        KeyboardController.close()
        const carry = parseFloat(clubCarry.text)
        const type = String(clubType.currentValue)
        const success = adding
            ? app.addClub(clubName.text, carry, type,
                          recommendationSwitch.checked)
            : app.updateClub(editId, clubName.text, carry, type,
                             recommendationSwitch.checked)
        if (!success)
            return
        const savedId = editId
        setBaseline()
        close()
        saved(adding, savedId)
    }

    parent: Overlay.overlay
    x: parent ? Math.round((parent.width - width) / 2) : 12
    y: parent ? Theme.statusHeight + 8 : 30
    width: parent ? parent.width - 24 : 776
    height: 226
    modal: true
    focus: true
    popupType: Popup.Item
    padding: 0
    closePolicy: Popup.NoAutoClose

    onClosed: KeyboardController.close()

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
                property: "scale"
                from: 0.985
                to: 1
                duration: Theme.motionSlow
                easing.type: Easing.OutBack
                easing.overshoot: 0.45
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
                property: "scale"
                from: 1
                to: 0.99
                duration: Theme.motionFast
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

    contentItem: Item {
        anchors.fill: parent
        anchors.margins: 16

        Text {
            id: titleText
            anchors.left: parent.left
            anchors.top: parent.top
            text: sheet.adding ? qsTr("Add club") : qsTr("Club details")
            color: Theme.text
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(21)
        }

        IconButton {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: -8
            iconSource: "../../assets/icons/lucide/x.svg"
            iconColor: Theme.text
            accessibleName: qsTr("Close club details")
            transparent: true
            onClicked: sheet.requestClose()
        }

        Row {
            id: firstRow
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: titleText.bottom
            anchors.topMargin: 8
            height: 67
            spacing: 12

            Column {
                width: 246
                spacing: 3
                Text {
                    text: qsTr("Club type")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(11)
                }
                AppSelectSheet {
                    id: clubType
                    width: parent.width
                    model: sheet.typeOptions
                }
            }

            Column {
                width: firstRow.width - 258
                spacing: 3
                Text {
                    text: qsTr("Club name")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(11)
                }
                AppTextField {
                    id: clubName
                    width: parent.width
                    placeholderText: qsTr("Example: 7 iron")
                    onAccepted: sheet.commit()
                }
            }
        }

        Row {
            id: secondRow
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: firstRow.bottom
            anchors.topMargin: 7
            height: 67
            spacing: 12

            Column {
                width: 224
                spacing: 3
                Text {
                    text: app.metric ? qsTr("Carry (metres)")
                                     : qsTr("Carry (yards)")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(11)
                }
                AppTextField {
                    id: clubCarry
                    width: parent.width
                    placeholderText: "145"
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    numericKeyboard: true
                    onAccepted: sheet.commit()
                }
            }

            Item {
                width: 242
                height: parent.height
                Text {
                    anchors.left: parent.left
                    anchors.right: recommendationSwitch.left
                    anchors.rightMargin: 6
                    anchors.verticalCenter: recommendationSwitch.verticalCenter
                    text: qsTr("Active")
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.DemiBold
                    font.pixelSize: Theme.px(13)
                    elide: Text.ElideRight
                }
                AppSwitch {
                    id: recommendationSwitch
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    accessibleName: qsTr("Active")
                }
            }

            Row {
                width: secondRow.width - 490
                height: parent.height
                spacing: 8
                AppButton {
                    width: (parent.width - 8) / 2
                    text: qsTr("Cancel")
                    variant: "surface"
                    compact: true
                    onClicked: sheet.requestClose()
                }
                AppButton {
                    width: (parent.width - 8) / 2
                    text: sheet.adding ? qsTr("Add club") : qsTr("Save club")
                    variant: "primary"
                    compact: true
                    enabled: sheet.draftValid
                    onClicked: sheet.commit()
                }
            }
        }
    }

    ConfirmDialog {
        id: discardDialog
        parent: Overlay.overlay
        title: qsTr("Discard club details?")
        bodyText: qsTr("Your unsaved club details will be lost.")
        confirmText: qsTr("Discard")
        onConfirmed: {
            sheet.setBaseline()
            sheet.close()
        }
    }
}
