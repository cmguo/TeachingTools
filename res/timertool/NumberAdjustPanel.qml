import QtQuick 2.7
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import "qrc:/uibase/qml/talwidget/TalConstant.js" as TalConstant
import TalDisplay 1.0

import "./"

Rectangle {
    property int num: 0
    property int maxNum: 9
    property bool canAdjustNum: true

    border.width: canAdjustNum ? Destiny.dp(2) : 0
    border.color: "#ECECEC"
    radius: width / 2;
    Item {
        width: parent.width
        height: parent.height

        AddOrSubButton {
            id:add
            width: parent.width
            height: Destiny.dp(32)
            anchors { top: parent.top; topMargin: Destiny.dp(26) }
            visible: canAdjustNum
            isAdd: true
            enabled: num < maxNum

            onClick: {
                num++
            }
        }

        AddOrSubButton {
            id: dec
            width: parent.width
            height: Destiny.dp(32)
            anchors { bottom: parent.bottom; bottomMargin: Destiny.dp(26) }
            visible: canAdjustNum
            isAdd: false
            enabled: num > 0

            onClick: {
                num--
            }
        }

        Text {
            id: number
            width: parent.width
            height: canAdjustNum ? parent.height - add.height - add.anchors.topMargin - dec.height - dec.anchors.bottomMargin : parent.height
            anchors { top: add.bottom; bottom: dec.top }
            font.pixelSize: Destiny.sp(120)
            color: "#2B2B2B"
            font.bold: true
            text: num
            font.family: TalConstant.font
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}



