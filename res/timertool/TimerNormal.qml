import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "qrc:/uibase/qml/talwidget/TalConstant.js" as TalConstant
import TalDisplay 1.0

import "qrc:/uibase/qml/talwidget/TalColor.js" as TalColors
import "qrc:/uibase/qml/talwidget"
import "qrc:/uibase/qml/talwidget/styles"

Rectangle{
    id:timeNomal
    property int totalTime: 0
    signal runingClick();
    signal closeBtnClick();

    border.color: "#E2E3E4"
    border.width: Destiny.dp(1)
    color: "#FAFAFA"
    Rectangle{
        id:topRect
        width: parent.width - anchors.leftMargin - anchors.rightMargin
        height: Destiny.dp(64)
        radius: Destiny.dp(8)
        anchors.top: parent.top
        anchors.topMargin: Destiny.dp(1)
        anchors.left: parent.left
        anchors.leftMargin: Destiny.dp(1)
        anchors.right: parent.right
        anchors.rightMargin: Destiny.dp(1)
        color: "#FAFAFA"

        Rectangle {
            width: parent.width
            height: Destiny.dp(8)
            anchors.bottom: parent.bottom
            color: "#FAFAFA"
        }

        TalTabBar {
            id: tabBar
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height
            spacing: Destiny.dp(48)

            model: countdownModel
            delegate: TalButton {
                id: itemBtn
                height: parent.height

                property bool isCurrentItem: {
                    return ListView.isCurrentItem
                }
                talStyle: TalButtonStyleTabBar {
                    fontPixelSize: Destiny.sp(18)
                    textColor: itemBtn.isCurrentItem ? TalColors.colorConfigBrandPrimary : TalColors.colorGray900
                }
                text: name
                onClicked: {
                    tabBar.currentIndex = index
                }
            }

            ListModel {
                id: countdownModel
                ListElement {
                    name: "倒计时"
                    enabled: true
                }
                ListElement {
                    name: "正计时"
                    enabled: true
                }
            }

            TalTabBarIndicator {
                anchors.bottom: parent.bottom
                color: TalColors.colorConfigBrandSecondary
            }
        }

        Button {
            width: Destiny.dp(32)
            height: Destiny.dp(32)
            anchors.right: parent.right
            anchors.rightMargin :  Destiny.dp(12)
            anchors.verticalCenter: parent.verticalCenter
            background: Image{
                source: "qrc:/teachingtools/timertool/close.png"
                fillMode:Image.PreserveAspectFit
            }
            onClicked: timeNomal.closeBtnClick()
        }
    }

    Rectangle {
        id: divider
        color: "#E2E3E4"
        width: parent.width
        height: Destiny.dp(1)
        anchors.top: topRect.bottom
    }

    StackLayout{
        id: swipeView
        anchors.top: divider.bottom
        anchors.left: parent.left
        anchors.leftMargin: Destiny.dp(1)
        anchors.right: parent.right
        anchors.rightMargin: Destiny.dp(1)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Destiny.dp(104)
        currentIndex: tabBar.currentIndex
        property int rowItemWidth: Destiny.dp(84)

        Rectangle{
            id:anticlockwise
            width: parent.width
            height: parent.height
            color: "#FAFAFA"
            opacity: 1.0
            Row{
                anchors.centerIn: parent
                height: Destiny.dp(260)
                spacing: Destiny.dp(10)
                NumberAdjustPanel {
                    id:minius1
                    width: swipeView.rowItemWidth
                    height: parent.height
                }
                NumberAdjustPanel {
                    id:minius2
                    width: swipeView.rowItemWidth
                    height: parent.height
                    num: 3
                }
                Text {
                    height: parent.height
                    width: Destiny.dp(40)
                    font.pixelSize: Destiny.dp(120)
                    verticalAlignment:Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.baseline: parent.verticalCenter
                    anchors.baselineOffset: font.pixelSize/3.5
                    text: qsTr(":")
                    color: "#2b2b2b"
                    font.bold: true
                    font.family: TalConstant.font
                }

                NumberAdjustPanel {
                    id:seconds1
                    width: swipeView.rowItemWidth
                    height: parent.height
                    maxNum: 5
                }
                NumberAdjustPanel {
                    id:seconds2
                    width: swipeView.rowItemWidth
                    height: parent.height
                }
            }
            Behavior on opacity {
                NumberAnimation { duration: 1000 }
            }
        }
        Rectangle{
            id:clockwise
            width: parent.width
            height: parent.height
            color: "#FAFAFA"
            opacity: 0.0
            Row{
                anchors.centerIn: parent
                height: Destiny.dp(260)
                spacing: Destiny.dp(10)
                NumberAdjustPanel {
                    width: swipeView.rowItemWidth
                    height: parent.height
                    canAdjustNum: false
                }
                NumberAdjustPanel {
                    width: swipeView.rowItemWidth
                    height: parent.height
                    canAdjustNum: false
                }

                Text {
                    height: parent.height
                    width: Destiny.dp(40)
                    font.pixelSize: Destiny.dp(120)
                    verticalAlignment:Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.baseline: parent.verticalCenter
                    anchors.baselineOffset: font.pixelSize/3.5
                    text: qsTr(":")
                    color: "#2b2b2b"
                    font.bold: true
                    font.family: TalConstant.font
                }
                NumberAdjustPanel {
                    width: swipeView.rowItemWidth
                    height: parent.height
                    canAdjustNum: false
                }
                NumberAdjustPanel {
                    width: swipeView.rowItemWidth
                    height: parent.height
                    canAdjustNum: false
                }
            }
            Behavior on opacity {
                NumberAnimation { duration: 1000 }
            }
        }
        onCurrentIndexChanged: {
            switch (currentIndex) {
            case 0:
                anticlockwise.opacity = 1.0;
                clockwise.opacity = 0.0;
                break;
            case 1:
                anticlockwise.opacity = 0.0;
                clockwise.opacity = 1.0;
                break;

            }
        }

    }

    TalButton {
        id:startTimerBtn
        talStyle: TalButtonStylePrimary { size: TalButtonStyle.Size.L }
        width: Destiny.dp(256)
        anchors.horizontalCenter:   parent.horizontalCenter
        text: "开始计时"
        anchors.bottomMargin:  Destiny.dp(40)
        anchors.bottom: parent.bottom
        onClicked: {
            setTotalTime();
            if(totalTime !=0||swipeView.currentIndex==1)
                timeNomal.runingClick();
        }
    }



    function setTotalTime(){
        switch(swipeView.currentIndex){
        case 0:
            totalTime = (minius1.num*10+minius2.num)*60+seconds1.num*10+seconds2.num;
            break;
        case 1:
            totalTime = 0;
            break;
        }
    }

}
