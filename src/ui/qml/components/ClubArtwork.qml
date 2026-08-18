import QtQuick
import QtQuick.Effects
import OpenCaddie

Item {
    id: artwork

    property string clubType: "other"
    property bool selected: false
    property bool clubEnabled: true

    readonly property string normalizedType:
        ["driver", "wood", "hybrid", "iron", "wedge", "putter", "other"]
            .indexOf(clubType) >= 0 ? clubType : "other"

    Image {
        id: image
        anchors.fill: parent
        source: "../../assets/club-heads/" + artwork.normalizedType + ".png"
        sourceSize.width: 512
        sourceSize.height: 512
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
        opacity: artwork.clubEnabled ? 1 : 0.34
        scale: artwork.selected ? 1.055 : 1
        layer.enabled: artwork.selected
        layer.effect: MultiEffect {
            colorization: 1
            colorizationColor: Theme.fairway
            shadowEnabled: true
            shadowColor: Theme.fairway
            shadowOpacity: 0.45
            shadowBlur: 0.68
        }

        Behavior on opacity {
            NumberAnimation { duration: Theme.motionFast }
        }
        Behavior on scale {
            NumberAnimation {
                duration: Theme.motion
                easing.type: Easing.OutBack
                easing.overshoot: 0.55
            }
        }
    }
}
