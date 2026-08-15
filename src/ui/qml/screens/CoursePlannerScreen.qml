import QtQuick
import QtQuick.Controls
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    property var planHoles: app.coursePlan.holes || []

    Component.onCompleted: {
        if (planHoles.length === 0 && app.courses.length > 0)
            app.planCourse(app.courses[0].slug)
    }

    TopBar {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        title: qsTr("Course analyzer")
        subtitle: app.selectedCourseAnalyzedHoleCount > 0
                  ? qsTr("%1 · %2 holes saved")
                        .arg(app.coursePlan.name || app.selectedCourseName)
                        .arg(app.selectedCourseAnalyzedHoleCount)
                  : app.coursePlan.name || app.selectedCourseName
    }

    ListView {
        id: holeList
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: attribution.top
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        anchors.bottomMargin: 4
        clip: true
        model: root.planHoles
        boundsBehavior: Flickable.StopAtBounds
        spacing: 0

        delegate: Rectangle {
            id: row
            required property var modelData
            width: holeList.width
            height: 58
            color: rowTap.pressed ? Theme.controlPressed : "transparent"
            Accessible.role: Accessible.Button
            Accessible.name: qsTr("Hole %1").arg(modelData.number)

            Behavior on color {
                ColorAnimation { duration: Theme.motionFast }
            }

            Text {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: 54
                text: modelData.number
                color: Theme.amber
                font.family: "Inter"
                font.weight: Font.Bold
                font.pixelSize: Theme.px(30)
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 72
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("PAR %1  •  INDEX %2")
                      .arg(modelData.par).arg(modelData.index)
                color: Theme.text
                font.family: "Inter"
                font.weight: Font.DemiBold
                font.letterSpacing: 0.5
                font.pixelSize: Theme.px(13)
            }

            Text {
                anchors.right: openArrow.left
                anchors.rightMargin: 18
                anchors.verticalCenter: parent.verticalCenter
                text: app.distanceText(Number(modelData.lengthMetres || 0))
                color: Theme.fairway
                font.family: "Inter"
                font.weight: Font.Bold
                font.pixelSize: Theme.px(17)
            }

            IconButton {
                id: openArrow
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                transparent: true
                iconSource: "../../assets/icons/lucide/chevron-right.svg"
                iconColor: Theme.textMuted
                accessibleName: qsTr("Open hole map")
                onClicked: {
                    app.plannerHole = Number(row.modelData.number)
                    app.navigateTo("CoursePlannerMapScreen")
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
                onTapped: {
                    app.plannerHole = Number(row.modelData.number)
                    app.navigateTo("CoursePlannerMapScreen")
                }
            }
        }

        ScrollIndicator.vertical: ScrollIndicator { }
    }

    Text {
        id: attribution
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: Theme.gutter
        anchors.rightMargin: Theme.gutter
        height: 18
        text: app.coursePlan.attribution || "© OpenStreetMap contributors"
        color: Theme.textMuted
        font.family: "Inter"
        font.pixelSize: Theme.px(8)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
