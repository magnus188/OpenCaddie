import QtQuick
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    property string pendingSlug
    property string pendingVersion
    property string pendingName

    function requestDelete(course) {
        if (course.slug === "opencaddie-demo") return
        pendingSlug = course.slug
        pendingVersion = course.version
        pendingName = course.name
        removeDialog.open()
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: app.coursePickerMode === "plan"
               ? qsTr("Course analyzer") : qsTr("Play golf")
        onBack: app.screen = "WelcomeScreen"
    }

    ListView {
        id: courseList
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: searchButton.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        anchors.bottomMargin: 12
        model: app.courses
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        delegate: Item {
            id: row
            required property var modelData
            property real dragStartX: 0
            width: ListView.view.width
            height: 62

            Rectangle {
                anchors.right: parent.right
                width: Math.max(96, -content.x)
                height: parent.height
                color: Theme.danger
                visible: row.modelData.slug !== "opencaddie-demo"
                Text {
                    anchors.centerIn: parent
                    text: qsTr("Delete")
                    color: "#FFFFFF"
                    font.family: "Inter"
                    font.weight: Font.DemiBold
                    font.pixelSize: Theme.px(15)
                }
                TapHandler { onTapped: root.requestDelete(row.modelData) }
            }

            Rectangle {
                id: content
                x: 0
                width: parent.width
                height: parent.height
                color: rowTap.pressed ? Theme.controlPressed : Theme.background

                Text {
                    anchors.left: parent.left
                    anchors.right: chevron.left
                    anchors.leftMargin: 14
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: row.modelData.name
                    color: Theme.text
                    font.family: "Inter"
                    font.weight: Font.Medium
                    font.pixelSize: Theme.px(17)
                    elide: Text.ElideRight
                }
                IconButton {
                    id: chevron
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    transparent: true
                    iconSource: "../../assets/icons/lucide/chevron-right.svg"
                    iconColor: Theme.textMuted
                    accessibleName: qsTr("Open %1").arg(row.modelData.name)
                    onClicked: app.activateCourse(row.modelData.slug)
                }
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: Theme.divider
                }
                TapHandler {
                    id: rowTap
                    gesturePolicy: TapHandler.ReleaseWithinBounds
                    onTapped: {
                        if (content.x < -1) content.x = 0
                        else app.activateCourse(row.modelData.slug)
                    }
                }
                DragHandler {
                    target: null
                    enabled: row.modelData.slug !== "opencaddie-demo"
                    xAxis.enabled: true
                    yAxis.enabled: false
                    onActiveChanged: {
                        if (active) row.dragStartX = content.x
                        else if (content.x < -row.width * 0.72) {
                            content.x = 0
                            root.requestDelete(row.modelData)
                        } else {
                            content.x = content.x < -48 ? -96 : 0
                        }
                    }
                    onTranslationChanged: {
                        content.x = Math.max(-row.width,
                                             Math.min(0, row.dragStartX + translation.x))
                    }
                }
                Behavior on x {
                    NumberAnimation { duration: Theme.motionSheet; easing.type: Easing.OutCubic }
                }
            }
        }
    }

    AppButton {
        id: searchButton
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        anchors.bottomMargin: 18
        text: qsTr("Search for more courses")
        variant: "surface"
        onClicked: app.screen = "CourseSearchScreen"
    }

    ConfirmDialog {
        id: removeDialog
        title: qsTr("Delete course?")
        bodyText: root.pendingName
        onConfirmed: app.removeCourse(root.pendingSlug, root.pendingVersion)
    }
}
