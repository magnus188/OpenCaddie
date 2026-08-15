pragma Singleton
import QtQuick

// Single app-wide on-screen keyboard state. Text fields call open() (AppTextField
// does this automatically on tap); Main.qml hosts the one OnScreenKeyboard
// instance bound to this target.
QtObject {
    property Item target: null
    property bool numeric: false
    readonly property bool active: target !== null
    readonly property int keyboardHeight: 208
    signal commitRequested()

    function open(field, numericMode) {
        numeric = numericMode === true
        target = field
    }

    function close() {
        if (target)
            target.focus = false
        target = null
        numeric = false
    }
}
