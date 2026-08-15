import QtQuick
import QtQuick.Controls
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    backgroundColor: Theme.focusBackground
    property bool drawerOpen: false

    function signedDelta(value) {
        var rounded = Math.round(value);
        if (rounded > 0)
            return "+" + rounded;
        if (rounded < 0)
            return "−" + Math.abs(rounded);
        return "0";
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
        anchors.bottom: bottomNav.top
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
}
