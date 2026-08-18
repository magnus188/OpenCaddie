import QtQuick
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    clip: true
    z: 90
    visible: motion.running
    enabled: false
    property string kind: ""
    property real progress: 0
    property int lastSequence: -1

    function bounded(value) {
        return Math.max(0, Math.min(1, value))
    }

    function flightOpacity(value) {
        return Math.min(1, value / 0.08, (1 - value) / 0.16)
    }

    Connections {
        target: app
        function onCelebrationChanged() {
            if (app.celebrationSequence === root.lastSequence)
                return
            root.lastSequence = app.celebrationSequence
            root.kind = app.celebrationKind
            motion.restart()
        }
    }

    NumberAnimation {
        id: motion
        target: root
        property: "progress"
        from: 0
        to: 1
        duration: root.kind === "eagle" ? 2400
                  : root.kind === "birdie" ? 2050 : 1550
        easing.type: Easing.Linear
    }

    // Birdies rise as a loose flock. Each silhouette follows its own curved
    // path and the articulated wings complete several beats during the climb.
    Repeater {
        model: 4
        delegate: FlyingBird {
            id: bird
            required property int index
            readonly property real delayedProgress: root.bounded(
                (root.progress - index * 0.052) / 0.82)

            eagle: false
            flightProgress: delayedProgress
            phase: index * 0.19
            visible: root.kind === "birdie" && delayedProgress > 0
                     && delayedProgress < 1
            x: root.width * (0.12 + index * 0.19)
               + delayedProgress * (84 - index * 10)
               + Math.sin(delayedProgress * Math.PI * 2 + index) * 11
            y: root.height + 12
               - delayedProgress * (root.height + 92 + index * 8)
               - Math.sin(delayedProgress * Math.PI) * (24 + index * 3)
            rotation: -62
                      + Math.sin(delayedProgress * Math.PI * 2 + index * 0.7) * 6
            scale: 0.72 + index * 0.075
            opacity: root.flightOpacity(delayedProgress) * 0.96
        }
    }

    // The eagle crosses the full display in one controlled bank. Its wider,
    // slower wingbeat is intentionally distinct from the birdie flock.
    FlyingBird {
        id: eagle
        eagle: true
        flightProgress: root.progress
        phase: 0.08
        visible: root.kind === "eagle"
        x: -width - 18 + root.progress * (root.width + width * 2 + 36)
        y: root.height * 0.37
           - Math.sin(root.progress * Math.PI) * 104
           + Math.sin(root.progress * Math.PI * 2) * 10
        rotation: -4 + Math.sin(root.progress * Math.PI * 2) * 7
        scale: 0.84 + Math.sin(root.progress * Math.PI) * 0.12
        opacity: root.flightOpacity(root.progress)
    }

    // Par keeps the existing celebratory language, with a more believable
    // burst: pieces rise, turn and fall under simulated gravity.
    Repeater {
        model: 20
        delegate: Rectangle {
            required property int index
            readonly property real localProgress: root.bounded(
                (root.progress - (index % 4) * 0.018) / 0.94)
            readonly property real horizontalVelocity:
                ((index * 83) % 270) - 135
            readonly property real verticalVelocity: 170 + (index % 6) * 24

            visible: root.kind === "par" && localProgress > 0
                     && localProgress < 1
            width: index % 4 === 0 ? 4 : 6
            height: index % 4 === 0 ? 6 : 12
            radius: index % 4 === 0 ? width / 2 : 2
            color: index % 3 === 0 ? Theme.fairway
                   : index % 3 === 1 ? Theme.amber : Theme.water
            x: root.width / 2 + horizontalVelocity * localProgress
            y: root.height * 0.72
               - verticalVelocity * localProgress
               + 205 * localProgress * localProgress
            rotation: localProgress * (230 + index * 23)
            opacity: Math.min(1, localProgress / 0.06,
                              (1 - localProgress) / 0.24)
        }
    }
}
