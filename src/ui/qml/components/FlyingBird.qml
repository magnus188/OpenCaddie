import QtQuick

Item {
    id: root
    property bool eagle: false
    property real flightProgress: 0
    property real phase: 0
    readonly property real flapCycles: eagle ? 3.25 : 5.4
    readonly property real wingSweep: Math.sin(
        (flightProgress * flapCycles + phase) * Math.PI * 2)

    width: eagle ? 164 : 58
    height: eagle ? 100 : 40

    Item {
        id: artwork
        anchors.fill: parent
        transform: Translate {
            y: root.wingSweep * (root.eagle ? 1.4 : 0.65)
        }

        Image {
            id: upperWing
            x: root.eagle ? 2 : 0
            y: 0
            width: root.eagle ? 98 : 36
            height: root.eagle ? 51 : 21
            source: root.eagle
                    ? "../../assets/wildlife/eagle-wing-upper.svg"
                    : "../../assets/wildlife/bird-wing-upper.svg"
            sourceSize: Qt.size(width * 2, height * 2)
            fillMode: Image.PreserveAspectFit
            smooth: true
            mipmap: true
            transformOrigin: Item.BottomRight
            rotation: root.eagle
                      ? -3 + root.wingSweep * 13
                      : -4 + root.wingSweep * 20
        }

        Image {
            id: lowerWing
            x: upperWing.x
            y: root.eagle ? 49 : 19
            width: upperWing.width
            height: upperWing.height
            source: root.eagle
                    ? "../../assets/wildlife/eagle-wing-lower.svg"
                    : "../../assets/wildlife/bird-wing-lower.svg"
            sourceSize: Qt.size(width * 2, height * 2)
            fillMode: Image.PreserveAspectFit
            smooth: true
            mipmap: true
            transformOrigin: Item.TopRight
            rotation: root.eagle
                      ? 3 - root.wingSweep * 13
                      : 4 - root.wingSweep * 20
        }

        Image {
            x: root.eagle ? 84 : 29
            y: root.eagle ? 34 : 13
            width: root.eagle ? 78 : 29
            height: root.eagle ? 32 : 15
            source: root.eagle
                    ? "../../assets/wildlife/eagle-body.svg"
                    : "../../assets/wildlife/bird-body.svg"
            sourceSize: Qt.size(width * 2, height * 2)
            fillMode: Image.PreserveAspectFit
            smooth: true
            mipmap: true
        }
    }
}
