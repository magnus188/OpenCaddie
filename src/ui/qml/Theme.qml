pragma Singleton
import QtQuick

QtObject {
    readonly property color background: app.darkMode ? "#121212" : "#F6F7F0"
    readonly property color surface: app.darkMode ? "#101211" : "#FFFFFF"
    readonly property color surfaceRaised: app.darkMode ? "#161B17" : "#EDF1E9"
    readonly property color border: app.darkMode ? "#2B332D" : "#D8DED4"
    readonly property color text: app.darkMode ? "#F7F8F2" : "#172017"
    readonly property color textMuted: app.darkMode ? "#A7ADA5" : "#607064"
    readonly property color fairway: "#2FCB63"
    readonly property color greenDeep: "#167B43"
    readonly property color amber: "#D48A35"
    readonly property color sand: "#E0C27A"
    readonly property color water: "#2BA7D7"
    readonly property color danger: "#D94D3E"
    readonly property color overlay: app.darkMode ? "#D9101211" : "#D9F6F7F0"

    readonly property int touch: 48
    readonly property int radius: 8
    readonly property int gutter: 16

    function px(value) {
        return Math.round(value * app.textScale)
    }
}
