import QtQuick

Item {
    id: root
    property string innerColor: "rgba(47,203,99,0.12)"
    property string middleColor: "rgba(47,203,99,0.05)"
    property string outerColor: "rgba(47,203,99,0)"
    opacity: 1

    SequentialAnimation on opacity {
        running: root.visible
        loops: 1
        NumberAnimation {
            from: 1
            to: 0.92
            duration: Theme.motionAmbient
            easing.type: Easing.InOutSine
        }
        NumberAnimation {
            from: 0.92
            to: 1
            duration: Theme.motionAmbient
            easing.type: Easing.InOutSine
        }
    }

    Canvas {
        id: glowCanvas
        anchors.fill: parent
        renderTarget: Canvas.Image
        antialiasing: true

        onPaint: {
            var context = getContext("2d")
            context.clearRect(0, 0, width, height)
            var radius = Math.max(width, height) * 0.58
            var gradient = context.createRadialGradient(
                        width * 0.52, height * 0.48, 0,
                        width * 0.52, height * 0.48, radius)
            gradient.addColorStop(0, root.innerColor)
            gradient.addColorStop(0.52, root.middleColor)
            gradient.addColorStop(1, root.outerColor)
            context.fillStyle = gradient
            context.fillRect(0, 0, width, height)
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    onInnerColorChanged: glowCanvas.requestPaint()
    onMiddleColorChanged: glowCanvas.requestPaint()
    onOuterColorChanged: glowCanvas.requestPaint()
}
