import QtQuick
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    property int draftStrokes: app.strokes > 0 ? app.strokes : app.par
    property int draftPutts: app.putts
    property int draftPenalties: app.penalties
    property string draftFairway: app.fairway
    property bool draftGir: app.gir
    property string draftNotes: app.notes
    property bool detailsOpen: false

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Hole %1 score").arg(app.currentHole)
        onBack: app.screen = "LiveHoleScreen"
    }

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: saveButton.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        anchors.bottomMargin: 8
        spacing: 0

        Item {
            width: parent.width
            height: 66
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: qsTr("Score"); color: Theme.text; font.family: "Inter"; font.weight: Font.DemiBold; font.pixelSize: Theme.px(18) }
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6
                IconButton { iconSource: "../../assets/icons/lucide/minus.svg"; accessibleName: qsTr("Decrease score"); onClicked: root.draftStrokes = Math.max(1, root.draftStrokes - 1) }
                Text { width: 70; height: Theme.touch; text: root.draftStrokes; color: Theme.amber; font.family: "Inter"; font.weight: Font.Bold; font.pixelSize: Theme.px(36); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                IconButton { iconSource: "../../assets/icons/lucide/plus.svg"; accessibleName: qsTr("Increase score"); onClicked: root.draftStrokes = Math.min(20, root.draftStrokes + 1) }
            }
            Rectangle { anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; height: 1; color: Theme.divider }
        }

        Item {
            width: parent.width
            height: 56
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: qsTr("Putts"); color: Theme.text; font.family: "Inter"; font.pixelSize: Theme.px(16) }
            Row { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                IconButton { iconSource: "../../assets/icons/lucide/minus.svg"; accessibleName: qsTr("Decrease putts"); onClicked: root.draftPutts = Math.max(0, root.draftPutts - 1) }
                Text { width: 52; height: Theme.touch; text: root.draftPutts; color: Theme.text; font.family: "Inter"; font.weight: Font.DemiBold; font.pixelSize: Theme.px(20); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                IconButton { iconSource: "../../assets/icons/lucide/plus.svg"; accessibleName: qsTr("Increase putts"); onClicked: root.draftPutts = Math.min(12, root.draftPutts + 1) }
            }
            Rectangle { anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; height: 1; color: Theme.divider }
        }

        Item {
            width: parent.width
            height: 56
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: qsTr("Penalties"); color: Theme.text; font.family: "Inter"; font.pixelSize: Theme.px(16) }
            Row { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; spacing: 6
                IconButton { iconSource: "../../assets/icons/lucide/minus.svg"; accessibleName: qsTr("Decrease penalties"); onClicked: root.draftPenalties = Math.max(0, root.draftPenalties - 1) }
                Text { width: 52; height: Theme.touch; text: root.draftPenalties; color: Theme.text; font.family: "Inter"; font.weight: Font.DemiBold; font.pixelSize: Theme.px(20); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                IconButton { iconSource: "../../assets/icons/lucide/plus.svg"; accessibleName: qsTr("Increase penalties"); onClicked: root.draftPenalties = Math.min(12, root.draftPenalties + 1) }
            }
            Rectangle { anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; height: 1; color: Theme.divider }
        }

        Item {
            width: parent.width
            height: 48
            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: qsTr("More details"); color: Theme.text; font.family: "Inter"; font.pixelSize: Theme.px(16) }
            IconButton { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; transparent: true; iconSource: "../../assets/icons/lucide/chevron-right.svg"; iconColor: Theme.textMuted; rotation: root.detailsOpen ? 90 : 0; accessibleName: qsTr("More details"); onClicked: root.detailsOpen = !root.detailsOpen; Behavior on rotation { NumberAnimation { duration: Theme.motion } } }
            TapHandler { onTapped: root.detailsOpen = !root.detailsOpen }
        }

        Column {
            width: parent.width
            height: root.detailsOpen ? 98 : 0
            visible: height > 0
            clip: true
            spacing: 4
            Behavior on height { NumberAnimation { duration: Theme.motionSheet; easing.type: Easing.OutCubic } }

            Row {
                width: parent.width
                height: Theme.touch
                spacing: 6
                Text { width: 90; height: parent.height; text: qsTr("Fairway"); color: Theme.textMuted; font.family: "Inter"; font.pixelSize: Theme.px(13); verticalAlignment: Text.AlignVCenter }
                Repeater {
                    model: [{label: qsTr("—"), value: ""}, {label: qsTr("Left"), value: "left"}, {label: qsTr("Centre"), value: "centre"}, {label: qsTr("Right"), value: "right"}]
                    AppButton { required property var modelData; width: 84; compact: true; text: modelData.label; variant: root.draftFairway === modelData.value ? "primary" : "surface"; onClicked: root.draftFairway = modelData.value }
                }
                AppButton { width: 92; compact: true; text: qsTr("GIR"); variant: root.draftGir ? "primary" : "surface"; onClicked: root.draftGir = !root.draftGir }
            }
            AppTextField {
                width: parent.width
                placeholderText: qsTr("Notes")
                text: root.draftNotes
                onTextChanged: root.draftNotes = text
            }
        }
    }

    AppButton {
        id: saveButton
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        anchors.bottomMargin: 14
        text: qsTr("Save score")
        variant: "primary"
        onClicked: {
            if (app.saveHoleScore(root.draftStrokes, root.draftPutts,
                                  root.draftPenalties, root.draftFairway,
                                  root.draftGir, root.draftNotes)) {
                app.screen = "LiveHoleScreen"
            }
        }
    }
}
