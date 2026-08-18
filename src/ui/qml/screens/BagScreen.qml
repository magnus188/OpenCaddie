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
    property string draggingId: ""
    property int dragStartIndex: -1
    property int dragTargetIndex: -1
    property real dragPressOffsetX: 0
    property real dragPointerViewportX: 0
    property real dragOffsetX: 0
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
    readonly property bool dragging: draggingId.length > 0
    readonly property real carryKeyboardLift:
        carryStepper.keyboardActive
        ? Math.max(0,
                   quickEditor.y + editorTopRow.y + carryStepper.y +
                   carryStepper.height + 12 -
                   (root.height - KeyboardController.keyboardHeight))
        : 0

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

    function syncRackModel() {
        rackVisualModel.clear()
        for (var i = 0; i < app.clubs.length; ++i) {
            const club = app.clubs[i]
            rackVisualModel.append({
                "clubId": String(club.id),
                "clubName": String(club.name),
                "clubCarry": Number(club.carry),
                "clubUnit": String(club.unit),
                "clubType": String(club.type || "other"),
                "clubEnabled": Boolean(club.enabled)
            })
        }
    }

    function resetDragState() {
        draggingId = ""
        dragStartIndex = -1
        dragTargetIndex = -1
        dragPressOffsetX = 0
        dragPointerViewportX = 0
        dragOffsetX = 0
    }

    function clearSelection() {
        resetDragState()
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

    function loadClub(index, positionView) {
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
        if (positionView !== false) {
            Qt.callLater(function() {
                if (index >= 0 && index < app.clubs.length)
                    clubRack.positionViewAtIndex(index, ListView.Contain)
            })
        }
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

    function beginClubDrag(clubId, visualIndex, pressOffsetX,
                           pointerViewportX) {
        const appIndex = findClubIndex(clubId)
        if (appIndex < 0 || visualIndex < 0 ||
                visualIndex >= rackVisualModel.count)
            return false
        if (draftDirty) {
            requestAction("reorder", -1)
            return false
        }
        if (selectedIndex !== appIndex)
            loadClub(appIndex, false)
        reorderMode = true
        draggingId = clubId
        dragStartIndex = visualIndex
        dragTargetIndex = visualIndex
        dragPressOffsetX = pressOffsetX
        dragPointerViewportX = pointerViewportX
        dragOffsetX = 0
        return true
    }

    function updateDragGeometry() {
        if (!dragging)
            return
        const contentPointerX = clubRack.contentX + dragPointerViewportX
        const span = 112 + clubRack.spacing
        const candidate = Math.max(0, Math.min(
            rackVisualModel.count - 1,
            Math.round((contentPointerX - dragPressOffsetX) / span)))
        if (candidate !== dragTargetIndex) {
            rackVisualModel.move(dragTargetIndex, candidate, 1)
            dragTargetIndex = candidate
        }
        dragOffsetX = contentPointerX - dragPressOffsetX -
                      dragTargetIndex * span
    }

    function updateClubDrag(pointerViewportX) {
        if (!dragging)
            return
        dragPointerViewportX = Math.max(
            0, Math.min(clubRack.width, pointerViewportX))
        updateDragGeometry()
    }

    function cancelClubDrag() {
        resetDragState()
        syncRackModel()
    }

    function finishClubDrag() {
        if (!dragging)
            return
        const target = dragTargetIndex
        var ids = []
        for (var i = 0; i < rackVisualModel.count; ++i)
            ids.push(rackVisualModel.get(i).clubId)
        const orderChanged = target !== dragStartIndex
        resetDragState()
        if (!orderChanged)
            return
        selectionAfterReloadIndex = target
        app.reorderClubs(ids)
    }

    function leaveReorderMode() {
        cancelClubDrag()
        reorderMode = false
    }

    Component.onCompleted: Qt.callLater(function() {
        root.syncRackModel()
        root.syncSelection()
        if (screenshotMode && app.clubs.length > 2)
            root.loadClub(2)
    })

    Connections {
        target: app
        function onClubsChanged() {
            root.resetDragState()
            root.syncRackModel()
            Qt.callLater(root.syncSelection)
        }
    }

    ListModel {
        id: rackVisualModel
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

        ListView {
            id: clubRack
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            orientation: ListView.Horizontal
            model: rackVisualModel
            clip: true
            interactive: !root.dragging
            boundsBehavior: Flickable.StopAtBounds
            flickDeceleration: 2200
            maximumFlickVelocity: 3000
            spacing: 2
            ScrollIndicator.horizontal: ScrollIndicator { }
            moveDisplaced: Transition {
                NumberAnimation {
                    properties: "x,y"
                    duration: Theme.motion
                    easing.type: Easing.OutCubic
                }
            }

            delegate: Item {
                id: clubDelegate
                required property int index
                required property string clubId
                required property string clubName
                required property real clubCarry
                required property string clubUnit
                required property string clubType
                required property bool clubEnabled
                property bool holdActivated: false
                width: 112
                height: clubRack.height
                opacity: root.draggingId === clubId
                         ? 1 : clubEnabled ? 1 : 0.68
                z: root.draggingId === clubId ? 100 : 0

                Item {
                    id: dragVisual
                    width: parent.width
                    height: parent.height
                    x: root.draggingId === clubDelegate.clubId
                       ? root.dragOffsetX : 0
                    y: root.draggingId === clubDelegate.clubId ? -4 : 0
                    scale: root.draggingId === clubDelegate.clubId
                           ? 1.08 : 1
                    transformOrigin: Item.Center

                    Behavior on scale {
                        NumberAnimation {
                            duration: Theme.motionFast
                            easing.type: Easing.OutCubic
                        }
                    }

                    Behavior on y {
                        NumberAnimation {
                            duration: Theme.motionFast
                            easing.type: Easing.OutCubic
                        }
                    }

                    Rectangle {
                        x: -5
                        y: 7
                        width: parent.width + 10
                        height: parent.height
                        radius: 16
                        color: Qt.rgba(0, 0, 0, 0.38)
                        visible: root.draggingId === clubDelegate.clubId
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 4
                        radius: 12
                        color: root.selectedId === clubDelegate.clubId
                               ? Qt.rgba(0.18, 0.80, 0.39, 0.075)
                               : clubPointer.pressed
                                 ? Theme.controlPressed : "transparent"
                        border.width:
                            root.draggingId === clubDelegate.clubId ? 2 : 0
                        border.color: Theme.fairway

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
                        clubType: clubDelegate.clubType
                        selected: root.selectedId === clubDelegate.clubId
                        clubEnabled: clubDelegate.clubEnabled
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.topMargin: 140
                        anchors.leftMargin: 5
                        anchors.rightMargin: 5
                        text: clubDelegate.clubName
                        color: root.selectedId === clubDelegate.clubId
                               ? Theme.fairway
                               : clubDelegate.clubEnabled
                                 ? Theme.text : Theme.textMuted
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
                        text: clubDelegate.clubEnabled
                              ? Math.round(clubDelegate.clubCarry) +
                                " " + clubDelegate.clubUnit
                              : qsTr("Inactive")
                        color: clubDelegate.clubEnabled
                               ? Theme.textMuted : Theme.amber
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
                        height: root.selectedId === clubDelegate.clubId
                                ? 4 : 0
                        radius: 2
                        color: Theme.fairway

                        Behavior on height {
                            NumberAnimation {
                                duration: Theme.motionFast
                                easing.type: Easing.OutCubic
                            }
                        }
                    }
                }

                MouseArea {
                    id: clubPointer
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    preventStealing:
                        root.draggingId === clubDelegate.clubId
                    pressAndHoldInterval: 500

                    onPressed: function(mouse) {
                        clubDelegate.holdActivated = false
                    }
                    onPressAndHold: function(mouse) {
                        if (root.dragging)
                            return
                        const point = clubDelegate.mapToItem(
                            clubRack, mouse.x, mouse.y)
                        clubDelegate.holdActivated = root.beginClubDrag(
                            clubDelegate.clubId, clubDelegate.index,
                            mouse.x, point.x)
                    }
                    onPositionChanged: function(mouse) {
                        if (root.draggingId !== clubDelegate.clubId)
                            return
                        const point = clubDelegate.mapToItem(
                            clubRack, mouse.x, mouse.y)
                        root.updateClubDrag(point.x)
                    }
                    onReleased: function(mouse) {
                        if (root.draggingId !== clubDelegate.clubId)
                            return
                        const point = clubDelegate.mapToItem(
                            clubRack, mouse.x, mouse.y)
                        root.updateClubDrag(point.x)
                        root.finishClubDrag()
                    }
                    onCanceled: {
                        if (root.draggingId === clubDelegate.clubId)
                            root.cancelClubDrag()
                    }
                    onClicked: {
                        if (!clubDelegate.holdActivated) {
                            root.requestAction(
                                "select",
                                root.findClubIndex(clubDelegate.clubId))
                        }
                    }
                }
            }

            Rectangle {
                x: root.dragTargetIndex * (112 + clubRack.spacing) + 4
                y: 4
                width: 104
                height: clubRack.height - 8
                radius: 12
                color: "transparent"
                border.width: 2
                border.color: Theme.fairway
                opacity: 0.72
                visible: root.dragging && root.dragTargetIndex >= 0
                z: 90
            }

            Timer {
                interval: 16
                repeat: true
                running: root.dragging
                onTriggered: {
                    const edge = 72
                    var delta = 0
                    if (root.dragPointerViewportX < edge) {
                        delta = -Math.ceil(
                            (edge - root.dragPointerViewportX) / edge * 8)
                    } else if (root.dragPointerViewportX >
                               clubRack.width - edge) {
                        delta = Math.ceil(
                            (root.dragPointerViewportX -
                             (clubRack.width - edge)) / edge * 8)
                    }
                    if (delta === 0)
                        return
                    const maximum = Math.max(
                        0, clubRack.contentWidth - clubRack.width)
                    const next = Math.max(
                        0, Math.min(maximum, clubRack.contentX + delta))
                    if (next !== clubRack.contentX) {
                        clubRack.contentX = next
                        root.updateDragGeometry()
                    }
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
        z: carryStepper.keyboardActive ? 5 : 0
        transform: Translate {
            y: -root.carryKeyboardLift

            Behavior on y {
                NumberAnimation {
                    duration: Theme.motion
                    easing.type: Easing.OutCubic
                }
            }
        }

        Item {
            anchors.fill: parent
            visible: root.hasSelection

            Item {
                id: normalEditor
                anchors.fill: parent
                visible: !root.reorderMode

                Item {
                    id: editorTopRow
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: normalActionRow.top
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    anchors.topMargin: 8
                    anchors.bottomMargin: 8

                    Row {
                        id: editorTopContent
                        anchors.fill: parent
                        spacing: 12

                        Item {
                            width: 184
                            height: parent.height

                            Text {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.topMargin: 10
                                text: root.draftName
                                color: Theme.text
                                font.family: "Inter"
                                font.weight: Font.Bold
                                font.pixelSize: Theme.px(21)
                                elide: Text.ElideRight
                            }

                            Text {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.topMargin: 52
                                text: root.clubTypeLabel(root.draftType)
                                color: Theme.textMuted
                                font.family: "Inter"
                                font.weight: Font.DemiBold
                                font.capitalization: Font.AllUppercase
                                font.letterSpacing: 0.8
                                font.pixelSize: Theme.px(10)
                                elide: Text.ElideRight
                            }
                        }

                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 1
                            height: 64
                            color: Theme.divider
                        }

                        Item {
                            width: 320
                            height: parent.height

                            Text {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.topMargin: 2
                                text: qsTr("Carry distance")
                                color: Theme.textMuted
                                font.family: "Inter"
                                font.weight: Font.DemiBold
                                font.pixelSize: Theme.px(11)
                            }

                            AppNumberStepper {
                                id: carryStepper
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.topMargin: 26
                                value: Math.round(root.draftCarry)
                                from: 1
                                to: app.metric ? 350 : 400
                                unit: app.metric ? qsTr("m") : qsTr("yd")
                                accessibleName: qsTr("Carry distance")
                                onValueEdited: function(newValue) {
                                    root.draftCarry = newValue
                                }
                            }
                        }

                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 1
                            height: 64
                            color: Theme.divider
                        }

                        Item {
                            width: editorTopContent.width - 506 -
                                   editorTopContent.spacing * 4
                            height: parent.height

                            Row {
                                anchors.centerIn: parent
                                spacing: 14

                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: qsTr("Active")
                                    color: Theme.text
                                    font.family: "Inter"
                                    font.weight: Font.DemiBold
                                    font.pixelSize: Theme.px(16)
                                }

                                AppSwitch {
                                    accessibleName: qsTr("Active")
                                    checked: root.draftEnabled
                                    onToggled: root.draftEnabled = checked
                                }
                            }
                        }
                    }
                }

                Row {
                    id: normalActionRow
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    anchors.bottomMargin: 12
                    height: Theme.touch
                    spacing: 12
                    property real unitWidth: (width - spacing * 2) / 4

                    AppButton {
                        width: parent.unitWidth * 2
                        height: parent.height
                        text: qsTr("Save changes")
                        iconSource: "../../assets/icons/lucide/check.svg"
                        variant: "primary"
                        compact: true
                        enabled: root.draftDirty
                        onClicked: root.saveQuickDraft()
                    }

                    AppButton {
                        width: parent.unitWidth
                        height: parent.height
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
                        width: parent.unitWidth
                        height: parent.height
                        text: qsTr("More")
                        variant: "surface"
                        compact: true
                        onClicked: clubActionsSheet.open()
                    }
                }
            }

            Item {
                id: reorderEditor
                anchors.fill: parent
                visible: root.reorderMode

                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: reorderDoneButton.top
                    anchors.leftMargin: 20
                    anchors.rightMargin: 20
                    anchors.topMargin: 12
                    anchors.bottomMargin: 8

                    Text {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        text: qsTr("Reorder bag")
                        color: Theme.text
                        font.family: "Inter"
                        font.weight: Font.Bold
                        font.pixelSize: Theme.px(19)
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.topMargin: 3
                        text: qsTr("Position %1 of %2")
                                  .arg((root.dragging
                                        ? root.dragTargetIndex
                                        : root.selectedIndex) + 1)
                                  .arg(app.clubs.length)
                        color: Theme.fairway
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(13)
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.topMargin: 39
                        text: root.dragging
                              ? qsTr("Release to place the club.")
                              : qsTr("Long-press a club, then drag it to a new position.")
                        color: Theme.textMuted
                        font.family: "Inter"
                        font.pixelSize: Theme.px(11)
                        elide: Text.ElideRight
                    }
                }

                AppButton {
                    id: reorderDoneButton
                    width: 188
                    height: Theme.touch
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.bottomMargin: 12
                    text: qsTr("Done")
                    iconSource: "../../assets/icons/lucide/check.svg"
                    variant: "primary"
                    compact: true
                    enabled: !root.dragging
                    onClicked: root.leaveReorderMode()
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

    ClubActionsSheet {
        id: clubActionsSheet
        onReorderRequested: root.requestAction("reorder", -1)
        onRemoveRequested: removeDialog.open()
    }

    ConfirmDialog {
        id: unsavedDialog
        title: qsTr("Discard changes?")
        bodyText: qsTr("Your unsaved club changes will be lost.")
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
        bodyText: qsTr("This permanently removes the club from your bag.")
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
