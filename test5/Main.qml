import QtQuick
import QtQuick.Window
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import QtQuick.Layouts
import Bigno 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("BGIS")

    Scene{
        id: scene
        anchors.fill: parent
        focus: true

        ToolBar {
            RowLayout {
                anchors.fill: parent

                ToolButton{
                    text:  "Open file"
                    onClicked: fileDialog.open();
                }

                ToolButton{
                    text:  "Fill Color"
                    onClicked: selezione.popup.visible = true;

                    ComboBox {
                        id: selezione
                        visible: false
                        displayText: "Fill color"
                    }
                }
            }
        }


        FileDialog {
            id: fileDialog
            title: "Please choose a file"
            fileMode: FileDialog.OpenFile
            nameFilters: ["Shape files (*.shp *.SHP)"]
            options: FileDialog.ReadOnly
            onAccepted: {
                //console.log(fileDialog.selectedFile)
                scene.selectedFile(fileDialog.selectedFile)
            }
            onRejected: {
                //console.log("Canceled")
            }
        }
    }
}
