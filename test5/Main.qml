import QtQuick
import QtQuick.Window
import Bigno 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Scene{
        anchors.fill: parent
        focus: true
    }

}
