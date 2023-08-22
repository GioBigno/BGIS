import QtQuick
import QtQuick.Window
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import Bigno 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Scene{
        id: scene
        anchors.fill: parent
        focus: true

        Button{
            text:  "Apri file"
            onClicked: fileDialog.open();
        }

        FileDialog {
            id: fileDialog
            title: "Please choose a file"
            fileMode: FileDialog.OpenFile
            nameFilters: ["Shape files (*.shp *.SHP)"]
            options: FileDialog.ReadOnly
            onAccepted: {
                console.log("You chose: " + fileDialog.selectedFile)
                scene.selectedFile(fileDialog.selectedFile)
            }
            onRejected: {
                console.log("Canceled")
            }

        }

    }






}
