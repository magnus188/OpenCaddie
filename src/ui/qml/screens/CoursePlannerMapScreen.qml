import QtQuick
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    backgroundColor: Theme.focusBackground
    property bool editing: true
    property bool routeDone: false
    property real pinchStartZoom: 1
    property real pinchStartRotation: 0
    property real panStartX: 0
    property real panStartY: 0
    property var planHoles: app.coursePlan.holes || []
    property var selectedData: findHole(app.plannerHole)

    function findHole(number) {
        for (var index = 0; index < planHoles.length; ++index) {
            if (Number(planHoles[index].number) === number)
                return planHoles[index]
        }
        return ({ teeX: 0, teeY: 0 })
    }

    function loadRoute() {
        if (!courseMap || !selectedData)
            return
        var saved = app.courseAnalysisLayups || []
        if (saved.length > 1) {
            courseMap.measurementPoints = saved
            editing = false
            routeDone = true
            return
        }
        courseMap.measurementPoints = [{
            x: Number(selectedData.teeX || 0),
            y: Number(selectedData.teeY || 0)
        }]
        editing = true
        routeDone = false
    }

    Component.onCompleted: loadRoute()
    onSelectedDataChanged: loadRoute()

    RadialMapGlow {
        anchors.fill: parent
        innerColor: app.darkMode ? "rgba(47,203,99,0.12)"
                                 : "rgba(22,123,67,0.10)"
        middleColor: app.darkMode ? "rgba(18,42,27,0.07)"
                                  : "rgba(47,203,99,0.04)"
    }

    CourseMap {
        id: courseMap
        anchors.fill: parent
        anchors.leftMargin: 56
        anchors.rightMargin: 56
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        modelSource: app.coursePlanMapSource
        hole: app.plannerHole
        colors: app.mapColors
        playerVisible: false
        metric: app.metric
        measurementFromPlayer: false
        measurementToTarget: false
        rotationDegrees: 0
        clip: true

        Behavior on rotationDegrees {
            NumberAnimation { duration: Theme.motionSheet; easing.type: Easing.OutCubic }
        }

        TapHandler {
            enabled: root.editing
            gesturePolicy: TapHandler.ReleaseWithinBounds
            onTapped: function(eventPoint) {
                var point = courseMap.mapPointAt(eventPoint.position.x,
                                                 eventPoint.position.y)
                if (!point.inside)
                    return
                var points = courseMap.measurementPoints.slice(0)
                points.push({ x: point.x, y: point.y })
                courseMap.measurementPoints = points
            }
        }

        DragHandler {
            target: null
            minimumPointCount: 1
            maximumPointCount: 1
            onActiveChanged: {
                if (active) {
                    root.panStartX = courseMap.panX
                    root.panStartY = courseMap.panY
                }
            }
            onTranslationChanged: {
                courseMap.panX = root.panStartX + translation.x
                courseMap.panY = root.panStartY + translation.y
            }
        }

        PinchHandler {
            target: null
            minimumPointCount: 2
            maximumPointCount: 2
            onActiveChanged: {
                if (active) {
                    root.pinchStartZoom = courseMap.zoom
                    root.pinchStartRotation = courseMap.rotationDegrees
                }
            }
            onActiveScaleChanged: courseMap.zoom = root.pinchStartZoom * activeScale
            onActiveRotationChanged: courseMap.rotationDegrees =
                                         root.pinchStartRotation + activeRotation
        }

        WheelHandler {
            target: null
            onWheel: function(event) {
                courseMap.zoom *= event.angleDelta.y > 0 ? 1.12 : 0.89
                event.accepted = true
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.surfaceRaised
        opacity: 0.94
        visible: !courseMap.ready && courseMap.errorText.length > 0
        z: 5
        Text {
            anchors.centerIn: parent
            width: 300
            text: courseMap.errorText
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(15)
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }

    IconButton {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 14
        anchors.topMargin: 10
        iconSource: "../../assets/icons/lucide/chevron-left.svg"
        iconColor: Theme.text
        accessibleName: qsTr("Back")
        onClicked: app.goBack()
        z: 10
    }

    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 76
        anchors.topMargin: 12
        spacing: -1
        z: 10

        Text {
            text: qsTr("HOLE")
            color: Theme.textMuted
            font.family: "Inter"
            font.weight: Font.DemiBold
            font.letterSpacing: 1.4
            font.pixelSize: Theme.px(9)
        }
        Text {
            text: app.plannerHole
            color: Theme.amber
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(27)
        }
    }

    IconButton {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 14
        anchors.topMargin: 10
        iconSource: "../../assets/icons/lucide/navigation.svg"
        iconColor: Theme.danger
        accessibleName: qsTr("North up")
        rotation: -courseMap.rotationDegrees
        onClicked: courseMap.rotationDegrees = 0
        z: 10
        Behavior on rotation {
            NumberAnimation { duration: Theme.motionSheet; easing.type: Easing.OutCubic }
        }
    }

    Column {
        anchors.right: parent.right
        anchors.rightMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8
        z: 10

        IconButton {
            iconSource: "../../assets/icons/lucide/plus.svg"
            iconColor: Theme.text
            accessibleName: qsTr("Zoom in")
            onClicked: courseMap.zoom *= 1.2
        }
        IconButton {
            iconSource: "../../assets/icons/lucide/minus.svg"
            iconColor: Theme.text
            accessibleName: qsTr("Zoom out")
            onClicked: courseMap.zoom /= 1.2
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 18
        width: Math.max(104, totalDistance.implicitWidth + 28)
        height: 42
        radius: 21
        color: Theme.overlay
        border.width: 1
        border.color: Theme.border
        visible: courseMap.measuredDistanceMetres > 0.5
        z: 10

        Text {
            id: totalDistance
            anchors.centerIn: parent
            text: app.distanceText(courseMap.measuredDistanceMetres)
            color: Theme.fairway
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(16)
        }
    }

    Row {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 18
        anchors.bottomMargin: 16
        spacing: 8
        visible: root.editing
        z: 10

        AppButton {
            text: qsTr("Undo")
            variant: "surface"
            compact: true
            enabled: courseMap.measurementPoints.length > 1
            onClicked: {
                var points = courseMap.measurementPoints.slice(0)
                points.pop()
                courseMap.measurementPoints = points
            }
        }
        AppButton {
            text: qsTr("Done")
            variant: "primary"
            compact: true
            enabled: courseMap.measurementPoints.length > 1
            onClicked: {
                if (app.saveCourseAnalysis(app.plannerHole,
                                           courseMap.measurementPoints)) {
                    root.editing = false
                    root.routeDone = true
                }
            }
        }
    }

    Row {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 18
        anchors.bottomMargin: 16
        spacing: 8
        visible: !root.editing
        z: 10

        AppButton {
            text: qsTr("Edit route")
            variant: "surface"
            compact: true
            onClicked: root.editing = true
        }
        AppButton {
            text: qsTr("Clear")
            variant: "surface"
            compact: true
            visible: root.routeDone
            onClicked: {
                if (app.clearCourseAnalysis(app.plannerHole))
                    root.loadRoute()
            }
        }
    }

    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 10
        anchors.bottomMargin: 6
        text: app.coursePlan.attribution || "© OpenStreetMap contributors"
        color: Theme.textMuted
        opacity: 0.82
        font.family: "Inter"
        font.pixelSize: Theme.px(8)
        z: 10
    }
}
