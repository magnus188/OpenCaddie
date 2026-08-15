import QtQuick
import QtQuick.Controls
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
        ScrollIndicator.vertical: ScrollIndicator { }

        delegate: Rectangle {
            id: row
            required property var modelData
            width: ListView.view.width
            height: 62
            color: rowTap.pressed ? Theme.controlPressed : Theme.background

            Text {
                anchors.left: parent.left
                anchors.right: actions.left
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

            Row {
                id: actions
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                IconButton {
                    visible: row.modelData.slug !== "opencaddie-demo"
                    transparent: true
                    iconSource: "../../assets/icons/lucide/trash-2.svg"
                    iconColor: Theme.danger
                    accessibleName: qsTr("Delete %1").arg(row.modelData.name)
                    onClicked: root.requestDelete(row.modelData)
                }
                IconButton {
                    transparent: true
                    iconSource: "../../assets/icons/lucide/chevron-right.svg"
                    iconColor: Theme.textMuted
                    accessibleName: qsTr("Open %1").arg(row.modelData.name)
                    onClicked: app.activateCourse(row.modelData.slug)
                }
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
                onTapped: app.activateCourse(row.modelData.slug)
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
        onClicked: app.navigateTo("CourseSearchScreen")
    }

    ConfirmDialog {
        id: removeDialog
        title: qsTr("Delete course?")
        bodyText: root.pendingName
        destructive: true
        confirmText: qsTr("Delete")
        onConfirmed: app.removeCourse(root.pendingSlug, root.pendingVersion)
    }
}
