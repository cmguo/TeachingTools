import QtQuick 2.12
import QtQuick.Controls 2.12

Button {
    id: control
    property color fontColor:control.checked?"white":"#2B2B2B"
    property color backgroudColor:control.checked?"#008FFF":"#E9E9E9";
    property Component backgroudGrident: null
    property int radius: 0
    property int borderWidth: 0
    property color  borderColor: "transparent"

    contentItem: Text {
        text: control.text
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        color: control.fontColor
    }
    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        color:control.backgroudColor
        radius: control.radius
        gradient: backgroudGrident
        border.width: borderWidth
        border.color: borderColor
    }

}
