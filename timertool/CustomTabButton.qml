import QtQuick 2.12
import QtQuick.Controls 2.12

TabButton {
    id: control
    z:checked?1:0
    property int fontSize: 16
    property color fontColor: control.checked?"white":"#2B2B2B"
    property color backGroundColor:control.checked?"#008FFF":"#E9E9E9"
    property int bottomBarWidth : 0
    property color bottomBarColor: "transparent"

    contentItem: Text {
        text: control.text
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        color: fontColor
    }
    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        color:backGroundColor
        radius: 8
        Rectangle{
            width: parent.width
            height: bottomBarWidth
            color:bottomBarColor
            anchors.bottom: parent.bottom
        }
    }
}
