import QtQuick
import QtQuick.Controls
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    backgroundColor: Theme.focusBackground
    property bool drawerOpen: false
    readonly property bool hasLastTrackedStroke:
        app.lastRecordedStroke && app.lastRecordedStroke.sequence !== undefined

    function signedDelta(value) {
        var rounded = Math.round(value);
        if (rounded > 0)
            return "+" + rounded;
        if (rounded < 0)
            return "−" + Math.abs(rounded);
        return "0";
    }

    function strokeTypeLabel(type) {
        if (type === "drive") return qsTr("Drive");
        if (type === "approach") return qsTr("Approach");
        if (type === "chip") return qsTr("Chip");
        if (type === "putt") return qsTr("Putt");
        return qsTr("Unknown");
    }

    function lastStrokeDetail() {
        if (!hasLastTrackedStroke)
            return "";
        return app.lastRecordedStroke.hasGps
            ? (app.lastRecordedStroke.distance !== undefined
                ? app.distanceText(app.lastRecordedStroke.distance)
                : qsTr("GPS"))
            : qsTr("No GPS");
    }

    function clubTypeForId(id) {
        for (var index = 0; index < app.clubs.length; ++index) {
            if (app.clubs[index].id === id)
                return app.clubs[index].type || "other";
        }
        return "other";
    }

    IconButton {
        id: menuButton
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 10
        anchors.topMargin: 6
        transparent: true
        iconSource: "../../assets/icons/lucide/menu.svg"
        iconColor: Theme.text
        accessibleName: qsTr("Round menu")
        onClicked: root.drawerOpen = true
        z: 10
    }

    HoleHeader {
        id: holeHeader
        anchors.horizontalCenter: pages.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 2
        hole: app.currentHole
        par: app.par
        strokeIndex: app.strokeIndex
        z: 5
    }

    SwipeView {
        id: pages
        anchors.left: parent.left
        anchors.right: liveMap.left
        anchors.top: holeHeader.bottom
        anchors.bottom: strokeTracker.top
        clip: true

        // Only the information pane participates in the carousel. The map is
        // a single, fixed sibling so it does not move during a page swipe.
        Item {
            id: distancePage

            Item {
                id: informationArea
                anchors.fill: parent

                Column {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: 4
                    width: 268
                    spacing: -6

                    Text {
                        width: parent.width
                        height: 66
                        text: app.distanceText(app.backDistance)
                        color: Theme.text
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(40)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    Text {
                        width: parent.width
                        height: 84
                        text: app.distanceText(app.centreDistance)
                        color: Theme.fairway
                        font.family: "Inter"
                        font.weight: Font.Bold
                        font.pixelSize: Theme.px(60)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        Behavior on color {
                            ColorAnimation {
                                duration: Theme.motion
                            }
                        }
                    }
                    Text {
                        width: parent.width
                        height: 66
                        text: app.distanceText(app.frontDistance)
                        color: Theme.text
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(40)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        // Page 2 — conditions and advice.
        Item {
            id: conditionsPage

            Item {
                id: conditionsInfo
                anchors.fill: parent

                Text {
                    anchors.centerIn: parent
                    width: parent.width - 96
                    visible: !app.weatherAvailable
                    text: qsTr("No weather data for this round")
                    color: Theme.textMuted
                    font.family: "Inter"
                    font.pixelSize: Theme.px(18)
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }

                Item {
                    anchors.fill: parent
                    visible: app.weatherAvailable

                    Row {
                        id: weatherContent
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.verticalCenterOffset: 4
                        spacing: 28

                        Column {
                            id: factorColumn
                            width: 140
                            spacing: 8

                            Item {
                                width: parent.width
                                height: 62
                                Text {
                                    width: 72
                                    height: parent.height
                                    text: root.signedDelta(app.playsLikeWindDelta)
                                    color: Theme.text
                                    font.family: "Inter"
                                    font.weight: Font.Bold
                                    font.pixelSize: Theme.px(42)
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                }
                                Image {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 84
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 42
                                    height: 36
                                    source: "../../assets/icons/figma/weather-wind.svg"
                                    sourceSize: Qt.size(96, 84)
                                    fillMode: Image.PreserveAspectFit
                                    rotation: app.windRelativeDegrees + 135
                                    smooth: true

                                    Behavior on rotation {
                                        NumberAnimation {
                                            duration: Theme.motionSlow
                                            easing.type: Easing.OutCubic
                                        }
                                    }
                                }
                            }

                            Item {
                                width: parent.width
                                height: 62
                                Text {
                                    width: 72
                                    height: parent.height
                                    text: root.signedDelta(app.playsLikeTemperatureDelta)
                                    color: Theme.text
                                    font.family: "Inter"
                                    font.weight: Font.Bold
                                    font.pixelSize: Theme.px(42)
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                }
                                Image {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 88
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 36
                                    height: 36
                                    source: "../../assets/icons/figma/weather-direction.svg"
                                    sourceSize: Qt.size(84, 84)
                                    fillMode: Image.PreserveAspectFit
                                    smooth: true
                                }
                            }

                            Item {
                                width: parent.width
                                height: 62
                                Text {
                                    width: 72
                                    height: parent.height
                                    text: root.signedDelta(app.playsLikeConditionDelta)
                                    color: Theme.text
                                    font.family: "Inter"
                                    font.weight: Font.Bold
                                    font.pixelSize: Theme.px(42)
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                }
                                Image {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 88
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 36
                                    height: 37
                                    source: "../../assets/icons/figma/weather-rain.svg"
                                    sourceSize: Qt.size(84, 86)
                                    fillMode: Image.PreserveAspectFit
                                    smooth: true
                                }
                            }

                            Text {
                                width: parent.width
                                visible: !app.playsLikeAvailable
                                text: qsTr("Waiting for GPS")
                                color: Theme.textMuted
                                font.family: "Inter"
                                font.pixelSize: Theme.px(13)
                                horizontalAlignment: Text.AlignRight
                            }
                        }

                        Item {
                            width: 160
                            height: factorColumn.height

                            Column {
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width
                                spacing: 12

                                Column {
                                    width: parent.width
                                    spacing: 0
                                    Text {
                                        width: parent.width
                                        text: qsTr("Plays like")
                                        color: Theme.text
                                        font.family: "Inter"
                                        font.weight: Font.Bold
                                        font.pixelSize: Theme.px(21)
                                        horizontalAlignment: Text.AlignRight
                                    }
                                    Text {
                                        width: parent.width
                                        height: 58
                                        text: app.playsLikeAvailable ? app.distanceText(app.playsLikeDistance) : "—"
                                        color: Theme.amber
                                        font.family: "Inter"
                                        font.weight: Font.Bold
                                        font.pixelSize: Theme.px(44)
                                        minimumPixelSize: Theme.px(24)
                                        fontSizeMode: Text.Fit
                                        horizontalAlignment: Text.AlignRight
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Column {
                                    width: parent.width
                                    spacing: 0
                                    Text {
                                        width: parent.width
                                        text: qsTr("Club")
                                        color: Theme.text
                                        font.family: "Inter"
                                        font.weight: Font.Bold
                                        font.pixelSize: Theme.px(21)
                                        horizontalAlignment: Text.AlignRight
                                    }
                                    Text {
                                        width: parent.width
                                        height: 58
                                        text: app.clubAdvice.length > 0 ? app.clubAdvice : "—"
                                        color: Theme.amber
                                        font.family: "Inter"
                                        font.weight: Font.Bold
                                        font.pixelSize: Theme.px(44)
                                        minimumPixelSize: Theme.px(21)
                                        fontSizeMode: Text.Fit
                                        horizontalAlignment: Text.AlignRight
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    StaticMapPreview {
        id: liveMap
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: bottomNav.top
        width: 306
        onOpenRequested: app.navigateTo("RoundMapScreen")
    }

    Item {
        id: strokeTracker
        anchors.left: parent.left
        anchors.right: liveMap.left
        anchors.bottom: bottomNav.top
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        height: 72
        z: 8

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 20
            text: qsTr("%1 tracked").arg(app.recordedStrokeCount) + " · "
                  + (app.shotGpsAvailable
                     ? qsTr("GPS will be saved")
                     : qsTr("No GPS — records anyway"))
            color: app.shotGpsAvailable ? Theme.textMuted : Theme.amber
            font.family: "Inter"
            font.pixelSize: Theme.px(11)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            height: Theme.touch
            spacing: 8

            Rectangle {
                id: lastStrokeChip
                visible: root.hasLastTrackedStroke
                enabled: app.canUndoRecordedStroke
                width: visible ? 160 : 0
                height: Theme.touch
                radius: Theme.radius
                color: lastStrokeTap.pressed
                       ? Theme.controlPressed : Theme.surfaceRaised
                border.width: 1
                border.color: Theme.border
                opacity: enabled ? 1 : 0.45
                Accessible.role: Accessible.Button
                Accessible.name: qsTr("Edit last recorded stroke type")
                Accessible.onPressAction: strokeTypeSheet.open()

                ClubArtwork {
                    id: lastClubArtwork
                    anchors.left: parent.left
                    anchors.leftMargin: 4
                    anchors.verticalCenter: parent.verticalCenter
                    width: 36
                    height: 42
                    visible: app.lastRecordedStroke.clubId !== undefined
                    clubType: root.clubTypeForId(app.lastRecordedStroke.clubId)
                }

                Column {
                    anchors.left: lastClubArtwork.visible
                                  ? lastClubArtwork.right : parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: lastClubArtwork.visible ? 2 : 10
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 1

                    Text {
                        width: parent.width
                        text: app.lastRecordedStroke.clubName !== undefined
                              ? app.lastRecordedStroke.clubName
                              : qsTr("Last stroke")
                        color: Theme.text
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(11)
                        elide: Text.ElideRight
                    }
                    Text {
                        width: parent.width
                        text: root.strokeTypeLabel(app.lastRecordedStroke.type) +
                              " · " + root.lastStrokeDetail()
                        color: Theme.textMuted
                        font.family: "Inter"
                        font.pixelSize: Theme.px(9)
                        elide: Text.ElideRight
                    }
                }

                TapHandler {
                    id: lastStrokeTap
                    enabled: lastStrokeChip.enabled
                    onTapped: strokeTypeSheet.open()
                }

                Behavior on color {
                    ColorAnimation { duration: Theme.motionFast }
                }
            }

            AppButton {
                visible: root.hasLastTrackedStroke
                width: visible ? 72 : 0
                height: Theme.touch
                text: qsTr("Undo")
                variant: "surface"
                compact: true
                enabled: app.canUndoRecordedStroke
                accessibleName: qsTr("Undo last recorded stroke")
                onClicked: app.undoLastRecordedStroke()
            }

            AppButton {
                width: root.hasLastTrackedStroke ? 176 : 232
                height: Theme.touch
                text: qsTr("Record stroke")
                variant: "primary"
                compact: true
                accessibleName: app.shotGpsAvailable
                    ? qsTr("Record stroke with GPS")
                    : qsTr("Record stroke without GPS")
                onClicked: clubPicker.open()
            }
        }
    }

    BottomNav {
        id: bottomNav
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        anchors.bottomMargin: 6
        pageCount: pages.count
        currentPage: pages.currentIndex
        onPrevious: app.previousHole()
        onNext: lastHole ? finishDialog.open() : app.nextHole()
        onPageSelected: page => pages.currentIndex = page
    }

    // Keep the required attribution close to the static map without competing
    // with the now-explicit navigation controls.
    Text {
        anchors.right: parent.right
        anchors.bottom: bottomNav.top
        anchors.rightMargin: 8
        anchors.bottomMargin: 2
        text: "© OpenStreetMap contributors"
        color: Theme.textMuted
        font.family: "Inter"
        font.pixelSize: Theme.px(7)
        opacity: 0.68
        z: 2
    }

    ScorePromptBanner {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 8
        z: 15
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.scrim
        opacity: root.drawerOpen ? 1 : 0
        visible: opacity > 0
        enabled: root.drawerOpen
        z: 20
        Behavior on opacity {
            NumberAnimation {
                duration: Theme.motionSheet
            }
        }
        TapHandler {
            onTapped: root.drawerOpen = false
        }
    }

    Rectangle {
        id: drawer
        x: root.drawerOpen ? 0 : -width
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 300
        color: Theme.surface
        border.width: 1
        border.color: Theme.border
        z: 21
        Behavior on x {
            NumberAnimation {
                duration: Theme.motionSheet
                easing.type: Easing.OutCubic
            }
        }

        Column {
            anchors.fill: parent
            anchors.margins: Theme.gutter
            spacing: 4

            Row {
                width: parent.width
                height: 58
                Text {
                    width: parent.width - closeDrawer.width
                    height: parent.height
                    text: app.courseName
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.Bold
                    font.pixelSize: Theme.px(20)
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                IconButton {
                    id: closeDrawer
                    anchors.verticalCenter: parent.verticalCenter
                    transparent: true
                    iconSource: "../../assets/icons/lucide/x.svg"
                    iconColor: Theme.text
                    accessibleName: qsTr("Close menu")
                    onClicked: root.drawerOpen = false
                }
            }
            Rectangle {
                width: parent.width
                height: 1
                color: Theme.divider
            }
            AppButton {
                width: parent.width
                text: qsTr("Enter score")
                variant: "surface"
                onClicked: {
                    root.drawerOpen = false;
                    app.navigateTo("HoleScoreScreen");
                }
            }
            AppButton {
                width: parent.width
                text: qsTr("Open full map")
                variant: "surface"
                onClicked: {
                    root.drawerOpen = false;
                    app.navigateTo("RoundMapScreen");
                }
            }
            AppButton {
                width: parent.width
                text: qsTr("Scorecard")
                variant: "surface"
                onClicked: {
                    root.drawerOpen = false;
                    app.navigateTo("ScorecardScreen");
                }
            }
            Item {
                width: 1
                height: 54
            }
            Rectangle {
                width: parent.width
                height: 1
                color: Theme.divider
            }
            AppButton {
                width: parent.width
                text: qsTr("Finish round")
                variant: "accent"
                onClicked: finishDialog.open()
            }
            AppButton {
                width: parent.width
                text: qsTr("Abandon round")
                variant: "danger"
                onClicked: abandonDialog.open()
            }
        }
    }

    ConfirmDialog {
        id: finishDialog
        title: qsTr("Finish round?")
        bodyText: qsTr("The round will be saved to history.")
        confirmText: qsTr("Finish")
        onConfirmed: app.finishRound()
    }
    ConfirmDialog {
        id: abandonDialog
        title: qsTr("Abandon round?")
        bodyText: qsTr("The active round will be closed.")
        destructive: true
        confirmText: qsTr("Abandon")
        onConfirmed: app.abandonRound()
    }

    ClubPickerSheet {
        id: clubPicker
        onClubSelected: clubId => {
            if (clubId.length > 0)
                app.recordStrokeWithClub(clubId);
            else
                app.recordStroke();
        }
    }

    Popup {
        id: strokeTypeSheet
        parent: Overlay.overlay
        x: parent ? (parent.width - width) / 2 : 75
        y: parent ? parent.height - height : 300
        width: Math.min(parent ? parent.width - 28 : 650, 650)
        height: 172
        modal: true
        focus: true
        padding: 14
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: Theme.surface
            radius: Theme.sheetRadius
            border.width: 1
            border.color: Theme.border
        }

        Text {
            id: typeSheetTitle
            anchors.left: parent.left
            anchors.top: parent.top
            text: qsTr("Correct stroke type")
            color: Theme.text
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(18)
        }

        IconButton {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: -8
            anchors.rightMargin: -8
            transparent: true
            iconSource: "../../assets/icons/lucide/x.svg"
            iconColor: Theme.text
            accessibleName: qsTr("Close stroke type picker")
            onClicked: strokeTypeSheet.close()
        }

        Row {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 82
            spacing: 8

            Repeater {
                model: [
                    { text: qsTr("Drive"), value: "drive", color: Theme.water },
                    { text: qsTr("Approach"), value: "approach", color: Theme.fairway },
                    { text: qsTr("Chip"), value: "chip", color: Theme.amber },
                    { text: qsTr("Putt"), value: "putt", color: Theme.text },
                    { text: qsTr("Unknown"), value: "unknown", color: Theme.textMuted }
                ]

                Button {
                    id: typeButton
                    required property var modelData
                    width: (parent.width - 32) / 5
                    height: parent.height
                    padding: 0
                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Set stroke type to %1").arg(modelData.text)
                    onClicked: {
                        if (app.setLastRecordedStrokeType(modelData.value))
                            strokeTypeSheet.close();
                    }

                    contentItem: Text {
                        text: typeButton.modelData.text
                        color: typeButton.modelData.color
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        font.pixelSize: Theme.px(12)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    background: Rectangle {
                        radius: Theme.radius
                        color: typeButton.down ? Theme.controlPressed
                                               : Theme.surfaceRaised
                        border.width: app.lastRecordedStroke.type ===
                                      typeButton.modelData.value ? 2 : 1
                        border.color: app.lastRecordedStroke.type ===
                                      typeButton.modelData.value
                                      ? typeButton.modelData.color : Theme.border
                    }
                }
            }
        }
    }
}
