import QtQuick
import OpenCaddie

Item {
    id: root
    anchors.fill: parent
    z: 90
    visible: motion.running
    enabled: false
    property string kind: ""
    property real progress: 0
    property int lastSequence: -1

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
        duration: root.kind === "eagle" ? 1700 : 1400
        easing.type: Easing.OutCubic
    }

    Repeater {
        model: 3
        delegate: Item {
            required property int index
            visible: root.kind === "birdie"
            width: 28
            height: 16
            x: 160 + index * 160 + root.progress * (index % 2 ? 45 : -30)
            y: 420 - root.progress * (380 + index * 24)
            opacity: Math.min(1, (1 - root.progress) * 2.2)
            rotation: index % 2 ? 8 : -8

            Rectangle {
                width: 16
                height: 3
                radius: 2
                color: Theme.fairway
                rotation: -24
                anchors.right: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
            Rectangle {
                width: 16
                height: 3
                radius: 2
                color: Theme.fairway
                rotation: 24
                anchors.left: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    Item {
        visible: root.kind === "eagle"
        width: 112
        height: 40
        x: -120 + root.progress * 1040
        y: 132 - root.progress * 36
        opacity: Math.min(1, (1 - root.progress) * 4)

        Rectangle {
            anchors.centerIn: parent
            width: 38
            height: 8
            radius: 4
            color: Theme.amber
        }
        Rectangle {
            width: 48
            height: 7
            radius: 4
            color: Theme.amber
            rotation: -18
            anchors.right: parent.horizontalCenter
            anchors.bottom: parent.verticalCenter
        }
        Rectangle {
            width: 48
            height: 7
            radius: 4
            color: Theme.amber
            rotation: 18
            anchors.left: parent.horizontalCenter
            anchors.bottom: parent.verticalCenter
        }
    }

    Repeater {
        model: 16
        delegate: Rectangle {
            required property int index
            visible: root.kind === "par"
            width: 5
            height: 10
            radius: 2
            color: index % 3 === 0 ? Theme.fairway
                   : index % 3 === 1 ? Theme.amber : Theme.water
            x: 90 + ((index * 47) % 620) + (index % 2 ? 24 : -18) * root.progress
            y: 400 - root.progress * (190 + (index % 5) * 24)
            rotation: root.progress * (180 + index * 19)
            opacity: Math.min(1, (1 - root.progress) * 2.5)
        }
    }
}
