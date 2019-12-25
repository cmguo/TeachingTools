import QtQuick 2.12
import QtQuick.Controls 2.12

TabButton {
    id: control
    z:checked?1:0
    contentItem: Text {
        text: control.text
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        color: control.checked?"white":"#2B2B2B"
    }
    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        color:control.checked?"#008FFF":"#E9E9E9";
        radius: 8
    }
}
