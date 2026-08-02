import QtQuick
import OpenCaddie

PageScaffold {
    id: root
    anchors.fill: parent
    backgroundColor: Theme.focusBackground
    property bool measuring: false
    property bool measurementDone: false
    property bool showingImportedAnalysis: false
    property real pinchStartZoom: 1
    property real pinchStartRotation: 0

    function beginMeasurement() {
        courseMap.measurementPoints = []
        measurementDone = false
        showingImportedAnalysis = false
        measuring = true
    }

    function loadImportedAnalysis() {
        var points = app.roundLayups || []
        courseMap.measurementPoints = points
        showingImportedAnalysis = points.length > 1
        measurementDone = showingImportedAnalysis
        measuring = false
    }

    Component.onCompleted: loadImportedAnalysis()

    Connections {
        target: app
        function onRoundChanged() { root.loadImportedAnalysis() }
    }

    RadialMapGlow {
        anchors.fill: parent
        innerColor: app.darkMode ? "rgba(47,203,99,0.11)"
                                 : "rgba(22,123,67,0.09)"
        middleColor: app.darkMode ? "rgba(18,42,27,0.06)"
                                  : "rgba(47,203,99,0.03)"
    }

    CourseMap {
        id: courseMap
        anchors.fill: parent
        modelSource: app.mapSource
        hole: app.currentHole
        colors: app.mapColors
        playerX: app.playerX
        playerY: app.playerY
        playerVisible: app.playerVisible
        metric: app.metric
        measurementFromPlayer: !root.showingImportedAnalysis && app.playerVisible
        measurementToTarget: false
        panX: 0
        panY: 0
        rotationDegrees: 0
        clip: true

        Behavior on rotationDegrees {
            NumberAnimation { duration: Theme.motionSheet; easing.type: Easing.OutCubic }
        }

        TapHandler {
            enabled: root.measuring
            gesturePolicy: TapHandler.ReleaseWithinBounds
            onTapped: function(eventPoint) {
                var point = courseMap.mapPointAt(eventPoint.position.x,
                                                 eventPoint.position.y)
                if (!point.inside) return
                var points = courseMap.measurementPoints.slice(0)
                points.push({x: point.x, y: point.y})
                courseMap.measurementPoints = points
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
        onClicked: app.screen = "LiveHoleScreen"
        z: 10
    }

    Text {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 76
        anchors.topMargin: 20
        text: qsTr("Hole %1").arg(app.currentHole)
        color: Theme.text
        font.family: "Inter"
        font.weight: Font.Bold
        font.pixelSize: Theme.px(20)
        z: 10
    }

    IconButton {
        id: northButton
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
        Behavior on rotation { NumberAnimation { duration: Theme.motionSheet; easing.type: Easing.OutCubic } }
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

    Rectangle {
        id: measurePill
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 18
        anchors.bottomMargin: 16
        width: 132
        height: 46
        radius: 23
        color: Theme.overlay
        border.width: 1
        border.color: Theme.border
        visible: !root.measuring && !root.measurementDone
        z: 10
        Row {
            anchors.centerIn: parent
            spacing: 6
            IconButton { width: 30; height: 30; padding: 4; transparent: true; iconSource: "../../assets/icons/lucide/ruler.svg"; iconColor: Theme.text; accessibleName: qsTr("Measure"); onClicked: root.beginMeasurement() }
            Text { anchors.verticalCenter: parent.verticalCenter; text: qsTr("Measure"); color: Theme.text; font.family: "Inter"; font.weight: Font.DemiBold; font.pixelSize: Theme.px(14) }
        }
        TapHandler {
            onTapped: root.beginMeasurement()
        }
    }

    Row {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 18
        anchors.bottomMargin: 16
        spacing: 8
        visible: root.measuring
        z: 10
        AppButton {
            text: qsTr("Undo")
            variant: "surface"
            compact: true
            enabled: courseMap.measurementPoints.length > 0
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
            onClicked: {
                root.measuring = false
                root.measurementDone = courseMap.measurementPoints.length > 0
            }
        }
    }

    AppButton {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 18
        anchors.bottomMargin: 16
        text: root.showingImportedAnalysis ? qsTr("Hide analysis") : qsTr("Clear")
        variant: "surface"
        compact: true
        visible: root.measurementDone && !root.measuring
        z: 10
        onClicked: {
            courseMap.measurementPoints = []
            root.measurementDone = false
            root.showingImportedAnalysis = false
        }
    }

    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 10
        anchors.bottomMargin: 6
        text: "© OpenStreetMap contributors"
        color: Theme.textMuted
        opacity: 0.82
        font.family: "Inter"
        font.pixelSize: Theme.px(8)
        z: 10
    }
}
