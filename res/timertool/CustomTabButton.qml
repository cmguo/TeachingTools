import QtQuick 2.12
import QtQuick.Controls 2.5
import TalDisplay 1.0
import Colors 1.0
import "qrc:/uibase/qml/talwidget/TalConstant.js" as TalConstant

TabButton {

    property int type: 0
    property string leftIcon: ""

    width: (visible ? implicitWidth : 0)
    height: parent.height

    rightPadding: 0
    leftPadding: 0

    topPadding: Destiny.dp(20)
    bottomPadding: Destiny.dp(20)

    font.pixelSize: Destiny.sp(18)
    font.family: TalConstant.font

    property bool talButtonActived: (this.down || this.checked)
    property int talButtonDuration: 100

    property string talColorActived: "#2B2B2B"
    property string talColorDisactived: "#7E7E7E"
    property string talColorDisabled: "#B6B6B6"

    contentItem: Text {
        id:contentText
        text: parent.text
        font: parent.font
        width: parent.width
        height: parent.height
        color: parent.enabled ? (talButtonActived ? talColorActived : talColorDisactived) : (talColorDisabled)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

        Behavior on color {
            PropertyAnimation {
                duration: 50
            }
        }
    }

    background: Item {
        implicitWidth: parent.width
        implicitHeight: parent.height

        Rectangle {
            width: parent.width - parent.parent.leftPadding
            anchors.left: parent.left
            anchors.leftMargin: parent.parent.leftPadding
            height: parent.height
            color: "transparent"

            Rectangle {
                width: parent.width
                height: Destiny.dp(4)
                anchors.bottom:  parent.bottom
                color: talButtonActived ? "#FF686E72" : "#00000000"
                Behavior on color {
                    PropertyAnimation {
                        duration: talButtonDuration
                    }
                }
            }
        }
    }
}
