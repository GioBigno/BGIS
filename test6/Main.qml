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
    title: qsTr("BGIS 6.0")

    ListModel {
        id: fillColorModel
        ListElement { text: "orange"}
        ListElement { text: "black"}
        ListElement { text: "white"}
        ListElement { text: "darkGray"}
        ListElement { text: "gray"}
        ListElement { text: "lightGray"}
        ListElement { text: "red"}
        ListElement { text: "green"}
        ListElement { text: "blue"}
        ListElement { text: "cyan"}
        ListElement { text: "magenta"}
        ListElement { text: "yellow"}
        ListElement { text: "darkRed"}
        ListElement { text: "darkGreen"}
        ListElement { text: "darkBlue"}
        ListElement { text: "darkCyan"}
        ListElement { text: "darkMagenta"}
    }

    Scene{
        id: scene
        anchors.fill: parent
        visible: true
        focus: true

        ToolBar {
            RowLayout {
                anchors.fill: parent

                ToolButton{
                    text:  "Open file"
                    onClicked: fileDialog.open();
                }

                ComboBox {
                    id: fillColorComboBox
                    visible: true
                    flat: true
                    displayText: "Fill color"
                    model: fillColorModel
                    Component.onCompleted: { currentIndex = 0
                                             scene.fillColor = currentValue
                                           }

                    onActivated: scene.fillColor = currentValue;
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
