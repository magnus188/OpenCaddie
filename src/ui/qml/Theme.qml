pragma Singleton
import QtQuick

QtObject {
    readonly property color background: app.darkMode ? "#101211" : "#F6F7F0"
    readonly property color focusBackground: app.darkMode ? "#0B0D0C" : "#F6F7F0"
    readonly property color surface: app.darkMode ? "#181B19" : "#FFFFFF"
    readonly property color surfaceRaised: app.darkMode ? "#202421" : "#EDF1E9"
    readonly property color border: app.darkMode ? "#2B302C" : "#D8DED4"
    readonly property color divider: border
    readonly property color text: app.darkMode ? "#F7F8F2" : "#172017"
    readonly property color textMuted: app.darkMode ? "#A7ADA5" : "#607064"
    readonly property color controlPressed: app.darkMode ? "#2A302B" : "#E5EAE2"
    readonly property color fairway: "#2FCB63"
    readonly property color greenDeep: "#167B43"
    readonly property color amber: "#D48A35"
    readonly property color sand: "#E0C27A"
    readonly property color water: "#2BA7D7"
    readonly property color danger: "#D94D3E"
    readonly property color overlay: app.darkMode ? "#F0181B19" : "#F0FFFFFF"
    readonly property color scrim: "#99000000"

    readonly property int touch: 48
    readonly property int radius: 8
    readonly property int sheetRadius: 14
    readonly property int statusHeight: 22
    readonly property int gutter: 24
    readonly property int navigationHeight: 64
    readonly property int motionFast: 140
    readonly property int motion: 180
    readonly property int motionSheet: 220

    readonly property int displayHole: 50
    readonly property int distancePrimary: 62
    readonly property int distanceSecondary: 48
    readonly property int screenTitle: 30
    readonly property int sectionTitle: 18
    readonly property int body: 14
    readonly property int caption: 11
    readonly property int control: 16

    function px(value) {
        return Math.round(value * app.textScale)
    }
}
