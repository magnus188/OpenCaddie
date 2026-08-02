import QtQuick
import OpenCaddie

Item {
    id: scaffold
    default property alias contentData: content.data
    property color backgroundColor: Theme.background

    Rectangle {
        anchors.fill: parent
        color: scaffold.backgroundColor
    }

    Item {
        id: content
        anchors.fill: parent
    }
}
