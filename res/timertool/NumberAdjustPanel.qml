﻿import QtQuick 2.7
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import "qrc:/uibase/qml/talwidget/TalConstant.js" as TalConstant
import TalDisplay 1.0


Rectangle {
    radius: 8;
    property int num: 0
    property int maxNum: 9
    property bool canAdjustNum: true
    color:canAdjustNum?"#F4F4F4":"transparent"
    border.width: canAdjustNum?Destiny.dp(2):0
    border.color: "#ECECEC"
    Column{
        width: parent.width
        height: parent.height
        Button {
            visible: canAdjustNum
            width: parent.width
            height: parent.height/4
            id:add
            font.pixelSize: Destiny.sp(40)
            font.family: TalConstant.font
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
            font.pixelSize: Destiny.sp(120)
            color: "#2B2B2B"
            font.bold: true
            text: num
            font.family: TalConstant.font
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Button {
            visible: canAdjustNum
            width: parent.width
            height: parent.height/4
            id:dec
            font.pixelSize: Destiny.sp(40)
            text: "-"
            enabled: num>0
            onClicked: num--;
            font.family: TalConstant.font
            background: Rectangle{
                color:"#00000000"
            }
        }
    }
}



