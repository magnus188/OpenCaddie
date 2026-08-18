import QtQuick
import QtQuick.Controls
import OpenCaddie

Item {
    id: root
    anchors.fill: parent

    property string selectedId: ""
    property int selectedIndex: -1
    property string draftName: ""
    property real draftCarry: 1
    property string draftType: "other"
    property bool draftEnabled: true
    property string baselineName: ""
    property real baselineCarry: 1
    property string baselineType: "other"
    property bool baselineEnabled: true
    property bool reorderMode: false
    property string pendingAction: ""
    property int pendingIndex: -1
    property int selectionAfterReloadIndex: -1

    readonly property bool hasSelection:
        selectedIndex >= 0 && selectedIndex < app.clubs.length &&
        selectedId.length > 0
    readonly property bool draftDirty:
        hasSelection && (draftName !== baselineName ||
                         Math.round(draftCarry) !== Math.round(baselineCarry) ||
                         draftType !== baselineType ||
                         draftEnabled !== baselineEnabled)

    function clubTypeLabel(type) {
        switch (type) {
        case "driver": return qsTr("Driver")
        case "wood": return qsTr("Fairway wood")
        case "hybrid": return qsTr("Hybrid")
        case "iron": return qsTr("Iron")
        case "wedge": return qsTr("Wedge")
        case "putter": return qsTr("Putter")
        default: return qsTr("Other")
        }
    }

    function findClubIndex(id) {
        for (var i = 0; i < app.clubs.length; ++i) {
            if (app.clubs[i].id === id)
                return i
        }
        return -1
    }

    function clearSelection() {
        selectedId = ""
        selectedIndex = -1
        draftName = ""
        draftCarry = 1
        draftType = "other"
        draftEnabled = true
        baselineName = ""
        baselineCarry = 1
        baselineType = "other"
        baselineEnabled = true
        reorderMode = false
    }

    function loadClub(index) {
        if (index < 0 || index >= app.clubs.length) {
            clearSelection()
            return
        }
        const club = app.clubs[index]
        selectedIndex = index
        selectedId = club.id
        draftName = club.name
        draftCarry = Math.max(1, Math.round(club.carry))
        draftType = club.type || "other"
        draftEnabled = club.enabled
        baselineName = draftName
        baselineCarry = draftCarry
        baselineType = draftType
        baselineEnabled = draftEnabled
        Qt.callLater(function() {
            if (index >= 0 && index < app.clubs.length)
                clubRack.positionViewAtIndex(index, ListView.Contain)
        })
    }

    function resetDraft() {
        draftName = baselineName
        draftCarry = baselineCarry
        draftType = baselineType
        draftEnabled = baselineEnabled
    }

    function syncSelection() {
        if (app.clubs.length === 0) {
            selectionAfterReloadIndex = -1
            clearSelection()
            return
        }
        var target = findClubIndex(selectedId)
        if (target < 0 && selectionAfterReloadIndex >= 0)
            target = Math.min(selectionAfterReloadIndex, app.clubs.length - 1)
        if (target < 0)
            target = 0
        selectionAfterReloadIndex = -1
        loadClub(target)
    }

    function performAction(action, index) {
        switch (action) {
        case "select":
            loadClub(index)
            break
        case "add":
            detailsSheet.openForAdd()
            break
        case "reorder":
            reorderMode = true
            break
        case "back":
            app.goBack()
            break
        }
    }

    function requestAction(action, index) {
        if (action === "select" && index === selectedIndex)
            return
        if (draftDirty) {
            pendingAction = action
            pendingIndex = index === undefined ? -1 : index
            unsavedDialog.open()
            return
        }
        performAction(action, index === undefined ? -1 : index)
    }

    function saveQuickDraft() {
        if (!hasSelection)
            return
        app.updateClub(selectedId, draftName, Math.round(draftCarry),
                       draftType, draftEnabled)
    }

    function moveSelected(delta) {
        if (!hasSelection)
            return
        const target = selectedIndex + delta
        if (target < 0 || target >= app.clubs.length)
            return
        var ids = []
        for (var i = 0; i < app.clubs.length; ++i)
            ids.push(app.clubs[i].id)
        const moved = ids.splice(selectedIndex, 1)[0]
        ids.splice(target, 0, moved)
        selectionAfterReloadIndex = target
        app.reorderClubs(ids)
    }

    Component.onCompleted: Qt.callLater(function() {
        root.syncSelection()
        if (screenshotMode && app.clubs.length > 2)
            root.loadClub(2)
    })

    Connections {
        target: app
        function onClubsChanged() {
            Qt.callLater(root.syncSelection)
        }
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("My bag")
        showBack: true
        autoBack: false
        actionText: qsTr("Add club")
        actionIconSource: "../../assets/icons/lucide/plus.svg"
        actionVariant: "accent"
        onBack: root.requestAction("back", -1)
        onActionTriggered: root.requestAction("add", -1)
    }

    Rectangle {
        id: rackPanel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        height: 204
        color: Theme.focusBackground

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: Theme.divider
        }

        Item {
            id: bagSlot
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 112

            Image {
                anchors.left: parent.left
                anchors.leftMargin: 4
                anchors.top: parent.top
                anchors.topMargin: 8
                width: 108
                height: 184
                source: "../../assets/club-heads/bag-rack.png"
                sourceSize.width: 512
                sourceSize.height: 512
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
            }

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 16
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 16
                width: 1
                color: Theme.border
            }
        }

        ListView {
            id: clubRack
            anchors.left: bagSlot.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.rightMargin: 4
            orientation: ListView.Horizontal
            model: app.clubs
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            flickDeceleration: 2200
            maximumFlickVelocity: 3000
            spacing: 2
            ScrollIndicator.horizontal: ScrollIndicator { }

            delegate: Item {
                id: clubDelegate
                required property var modelData
                required property int index
                width: 112
                height: clubRack.height
                opacity: modelData.enabled ? 1 : 0.68

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 4
                    radius: 12
                    color: root.selectedId === modelData.id
                           ? Qt.rgba(0.18, 0.80, 0.39, 0.075)
                           : clubTap.pressed ? Theme.controlPressed
                                             : "transparent"

                    Behavior on color {
                        ColorAnimation { duration: Theme.motionFast }
                    }
                }

                ClubArtwork {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 5
                    width: 96
                    height: 135
                    clubType: clubDelegate.modelData.type
                    selected: root.selectedId === clubDelegate.modelData.id
                    clubEnabled: clubDelegate.modelData.enabled
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 140
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    text: modelData.name
                    color: root.selectedId === modelData.id
                           ? Theme.fairway
                           : modelData.enabled ? Theme.text : Theme.textMuted
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
                    anchors.topMargin: 163
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    text: modelData.enabled
                          ? Math.round(modelData.carry) + " " + modelData.unit
                          : qsTr("Off")
                    color: modelData.enabled ? Theme.textMuted : Theme.amber
                    font.family: "Inter"
                    font.weight: Font.DemiBold
                    font.pixelSize: Theme.px(11)
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    height: root.selectedId === modelData.id ? 4 : 0
                    radius: 2
                    color: Theme.fairway

                    Behavior on height {
                        NumberAnimation {
                            duration: Theme.motionFast
                            easing.type: Easing.OutCubic
                        }
                    }
                }

                TapHandler {
                    id: clubTap
                    onTapped: root.requestAction("select", clubDelegate.index)
                }
            }

            Item {
                width: clubRack.width
                height: clubRack.height
                visible: app.clubs.length === 0

                Column {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - 48, 390)
                    spacing: 8
                    Text {
                        width: parent.width
                        text: qsTr("Your club rack is empty")
                        color: Theme.text
                        font.family: "Inter"
                        font.weight: Font.Bold
                        font.pixelSize: Theme.px(20)
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Text {
                        width: parent.width
                        text: qsTr("Add a club to start building your bag.")
                        color: Theme.textMuted
                        font.family: "Inter"
                        font.pixelSize: Theme.px(13)
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }

    Rectangle {
        id: quickEditor
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: rackPanel.bottom
        anchors.bottom: parent.bottom
        color: Theme.surface

        Item {
            anchors.fill: parent
            visible: root.hasSelection

            Item {
                id: clubSummary
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.leftMargin: 18
                width: 154

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 15
                    text: root.draftName
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(21)
                    elide: Text.ElideRight
                }

                Row {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: 49
                    spacing: 5
                    Text {
                        text: Math.round(root.draftCarry)
                        color: Theme.fairway
                        font.family: "Inter"
                        font.weight: Font.Bold
                        font.pixelSize: Theme.px(30)
                    }
                    Text {
                        anchors.baseline: parent.children[0].baseline
                        text: app.metric ? qsTr("m") : qsTr("yd")
                        color: Theme.textMuted
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(13)
                    }
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 94
                    text: root.clubTypeLabel(root.draftType)
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.weight: Font.DemiBold
                    font.capitalization: Font.AllUppercase
                    font.letterSpacing: 0.8
                    font.pixelSize: Theme.px(10)
                    elide: Text.ElideRight
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: 121
                    width: statusLabel.implicitWidth + 18
                    height: 27
                    radius: 14
                    color: root.draftEnabled
                           ? Qt.rgba(0.18, 0.80, 0.39, 0.13)
                           : Qt.rgba(0.83, 0.54, 0.21, 0.13)
                    border.width: 1
                    border.color: root.draftEnabled ? Theme.greenDeep : Theme.amber
                    Text {
                        id: statusLabel
                        anchors.centerIn: parent
                        text: root.draftEnabled ? qsTr("Recommended")
                                                : qsTr("Not recommended")
                        color: root.draftEnabled ? Theme.fairway : Theme.amber
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(10)
                    }
                }
            }

            Rectangle {
                anchors.left: clubSummary.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.topMargin: 16
                anchors.bottomMargin: 16
                width: 1
                color: Theme.divider
            }

            Item {
                id: carryEditor
                anchors.left: clubSummary.right
                anchors.leftMargin: 16
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 326
                visible: !root.reorderMode

                Row {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 13
                    Text {
                        text: qsTr("Carry distance")
                        color: Theme.text
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(13)
                    }
                    Text {
                        anchors.baseline: parent.children[0].baseline
                        width: parent.width - parent.children[0].implicitWidth
                        text: Math.round(root.draftCarry) + " " +
                              (app.metric ? qsTr("m") : qsTr("yd"))
                        color: Theme.fairway
                        font.family: "Inter"
                        font.weight: Font.Bold
                        font.pixelSize: Theme.px(13)
                        horizontalAlignment: Text.AlignRight
                    }
                }

                AppSlider {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 35
                    from: 1
                    to: app.metric ? 350 : 400
                    stepSize: 1
                    value: root.draftCarry
                    onMoved: root.draftCarry = Math.round(value)
                }

                Row {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 91
                    height: Theme.touch
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - recommendationSwitch.width
                        text: qsTr("Use for recommendations")
                        color: Theme.text
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(13)
                        elide: Text.ElideRight
                    }
                    AppSwitch {
                        id: recommendationSwitch
                        accessibleName: qsTr("Use for recommendations")
                        checked: root.draftEnabled
                        onToggled: root.draftEnabled = checked
                    }
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 145
                    text: qsTr("Fine tune in 1-unit steps")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(10)
                    elide: Text.ElideRight
                }
            }

            Item {
                anchors.left: clubSummary.right
                anchors.leftMargin: 16
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 326
                visible: root.reorderMode

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 16
                    text: qsTr("Reorder bag")
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(17)
                }
                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 45
                    text: qsTr("Move the selected club along the rack.")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(11)
                    elide: Text.ElideRight
                }
                Row {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 78
                    spacing: 10
                    AppButton {
                        width: (parent.width - 10) / 2
                        text: qsTr("Move left")
                        iconSource: "../../assets/icons/lucide/chevron-left.svg"
                        variant: "surface"
                        compact: true
                        enabled: root.selectedIndex > 0
                        onClicked: root.moveSelected(-1)
                    }
                    AppButton {
                        width: (parent.width - 10) / 2
                        text: qsTr("Move right")
                        iconSource: "../../assets/icons/lucide/chevron-right.svg"
                        variant: "surface"
                        compact: true
                        enabled: root.selectedIndex < app.clubs.length - 1
                        onClicked: root.moveSelected(1)
                    }
                }
            }

            Column {
                anchors.left: carryEditor.right
                anchors.leftMargin: 16
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.top: parent.top
                anchors.topMargin: 11
                spacing: 8
                visible: !root.reorderMode

                AppButton {
                    width: parent.width
                    text: qsTr("Save changes")
                    iconSource: "../../assets/icons/lucide/check.svg"
                    variant: "primary"
                    compact: true
                    enabled: root.draftDirty
                    onClicked: root.saveQuickDraft()
                }

                Row {
                    width: parent.width
                    spacing: 8
                    AppButton {
                        width: (parent.width - 8) / 2
                        text: qsTr("Details")
                        iconSource: "../../assets/icons/lucide/menu.svg"
                        accessibleName: qsTr("Edit details")
                        variant: "surface"
                        compact: true
                        onClicked: detailsSheet.openForEdit({
                            "id": root.selectedId,
                            "name": root.draftName,
                            "carry": root.draftCarry,
                            "type": root.draftType,
                            "enabled": root.draftEnabled
                        })
                    }
                    AppButton {
                        width: (parent.width - 8) / 2
                        text: qsTr("Remove")
                        iconSource: "../../assets/icons/lucide/trash-2.svg"
                        accessibleName: qsTr("Remove club")
                        variant: "danger"
                        compact: true
                        onClicked: removeDialog.open()
                    }
                }

                AppButton {
                    width: parent.width
                    text: qsTr("Reorder bag")
                    iconSource: "../../assets/icons/lucide/menu.svg"
                    variant: "surface"
                    compact: true
                    onClicked: root.requestAction("reorder", -1)
                }
            }

            Item {
                anchors.left: carryEditor.right
                anchors.leftMargin: 16
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: root.reorderMode

                AppButton {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 18
                    text: qsTr("Done reordering")
                    iconSource: "../../assets/icons/lucide/check.svg"
                    variant: "primary"
                    compact: true
                    onClicked: root.reorderMode = false
                }
                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 80
                    text: qsTr("Position %1 of %2")
                              .arg(root.selectedIndex + 1).arg(app.clubs.length)
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(12)
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
            }
        }

        Column {
            anchors.centerIn: parent
            width: Math.min(parent.width - 48, 430)
            spacing: 10
            visible: !root.hasSelection
            Text {
                width: parent.width
                text: qsTr("Ready for your first club?")
                color: Theme.text
                font.family: "Inter"
                font.weight: Font.Bold
                font.pixelSize: Theme.px(20)
                horizontalAlignment: Text.AlignHCenter
            }
            AppButton {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Add club")
                iconSource: "../../assets/icons/lucide/plus.svg"
                variant: "primary"
                compact: true
                onClicked: detailsSheet.openForAdd()
            }
        }
    }

    ConfirmDialog {
        id: unsavedDialog
        title: qsTr("Discard changes?")
        bodyText: qsTr("Your unsaved carry and recommendation changes will be lost.")
        confirmText: qsTr("Discard")
        onConfirmed: {
            root.resetDraft()
            const action = root.pendingAction
            const index = root.pendingIndex
            root.pendingAction = ""
            root.pendingIndex = -1
            root.performAction(action, index)
        }
    }

    ConfirmDialog {
        id: removeDialog
        title: qsTr("Remove club?")
        bodyText: qsTr("The club will no longer be available for recommendations.")
        destructive: true
        confirmText: qsTr("Remove")
        onConfirmed: {
            root.selectionAfterReloadIndex = Math.max(
                0, Math.min(root.selectedIndex, app.clubs.length - 2))
            app.removeClub(root.selectedId)
        }
    }

    ClubDetailsSheet {
        id: detailsSheet
        onSaved: function(added, clubId) {
            if (added && app.clubs.length > 0)
                root.loadClub(app.clubs.length - 1)
            else {
                const index = root.findClubIndex(clubId)
                if (index >= 0)
                    root.loadClub(index)
            }
        }
    }
}
