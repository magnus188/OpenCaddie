import QtQuick
import OpenCaddie

Item {
    id: bar
    property string title
    property string subtitle
    property bool showBack: true
    // When true (default) the back button pops the app back-stack; the back()
    // signal still fires first for screens that need side effects.
    property bool autoBack: true
    signal back()

    implicitHeight: Theme.navigationHeight

    IconButton {
        id: backButton
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        iconSource: "../../assets/icons/lucide/chevron-left.svg"
        iconColor: Theme.text
        accessibleName: qsTr("Back")
        transparent: true
        visible: bar.showBack
        onClicked: {
            bar.back()
            if (bar.autoBack)
                app.goBack()
        }
    }

    Column {
        anchors.left: showBack ? backButton.right : parent.left
        anchors.right: parent.right
        anchors.leftMargin: showBack ? 8 : 0
        anchors.verticalCenter: parent.verticalCenter
        spacing: 1

        Text {
            width: parent.width
            text: bar.title
            color: Theme.text
            font.family: "Inter"
            font.weight: Font.Bold
            font.pixelSize: Theme.px(Theme.screenTitle)
            elide: Text.ElideRight
        }
        Text {
            width: parent.width
            text: bar.subtitle
            color: Theme.textMuted
            font.family: "Inter"
            font.pixelSize: Theme.px(11)
            visible: text.length > 0
            elide: Text.ElideRight
        }
    }
}
