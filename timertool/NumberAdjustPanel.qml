import QtQuick 2.7
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4

Rectangle {
    radius: 8;
    property int num: 0
    property int maxNum: 9
    property bool canAdjustNum: true
    color:canAdjustNum?"#F4F4F4":"transparent"
    Column{
        width: parent.width
        height: parent.height
        Button {
            visible: canAdjustNum
            width: parent.width
            height: parent.height/4
            id:add
            font.pixelSize: 40
            text: "+"
            enabled: num < maxNum
            background: Rectangle{
                color:"#00000000"
            }

            onClicked: num++

        }
        Text {
            width: parent.width
            height: canAdjustNum?parent.height/2:parent.height

            id:number
            font.pixelSize: 120
            color: "#2B2B2B"
            text: num
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter


        }
        Button {
            visible: canAdjustNum
            width: parent.width
            height: parent.height/4
            id:dec
            font.pixelSize: 40
            text: "-"
            enabled: num>0
            onClicked: num--;
            background: Rectangle{
                color:"#00000000"
            }
        }
    }
}



