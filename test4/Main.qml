import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import Bigno 1.0

Window {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("Whiteboard")

    property alias pen: whiteboard.pen

    RowLayout{
        id: toolBar
        x: 10
        width: parent.width
        height: 50
        spacing: 10


        Rectangle{
            id: square
            width: 30
            height: 30
            color: "green"
            MouseArea{
                anchors.fill: parent
                onClicked: pen = "square"
            }
        }

        Rectangle{
            id: circle
            width: 30
            height: 30
            radius: 180
            color: "green"
            MouseArea{
                anchors.fill: parent
                onClicked: pen = "circle"
            }
        }

        Canvas {
                id: triangle
                width: 30; height: 30
                onPaint: {
                    var ctx = getContext("2d")
                    // the equliteral triangle
                    ctx.beginPath();
                    ctx.moveTo(30, 30);
                    ctx.lineTo(0, 30);
                    ctx.lineTo(15, 0);
                    ctx.closePath();
                    // fill color
                    ctx.fillStyle = "green";
                    ctx.fill();
                }
                MouseArea{
                    anchors.fill: parent
                    onClicked: pen = "triangle"
                }
            }
    }

    Whiteboard {
        id: whiteboard
        y: 50
        width: parent.width
        height: parent.height - 50


        MouseArea {
            anchors.fill: parent
            onClicked: whiteboard.drawPoint(mouseX, mouseY)
        }

    }
}
