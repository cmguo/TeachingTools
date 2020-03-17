import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Strings 1.0
import TalDisplay 1.0


Rectangle{
    id:timeNomal
    property int totalTime: 0
    signal runingClick();
    signal closeBtnClick();

    Rectangle{
        id:topRect
        width: parent.width
        radius: Destiny.dp(8)

        gradient: Gradient {
            GradientStop{ position: 0.0; color: "#F9F9F9";}
            GradientStop{ position: 1.0; color: "#ECECEC";}
        }

        height: Destiny.dp(64)

        TabBar {
            height: parent.height
            font.pointSize: Destiny.dp(14)
            width: implicitWidth
            anchors.centerIn:  parent
            id: tabBar
            currentIndex: 1
            spacing: Destiny.dp(48)
            background: Rectangle{
                color: "transparent"
            }
            CustomTabButton{
                text:qsTr("倒计时");
            }
            CustomTabButton{
                text:qsTr("正计时");
            }

            Component.onCompleted: {
                tabBar.currentIndex = 0;
            }

        }

        Button {
            width: Destiny.dp(40)
            height: Destiny.dp(40)
            anchors.right: parent.right
            anchors.rightMargin :  Destiny.dp(20)
            anchors.verticalCenter: parent.verticalCenter
            background: Image{
                source: "./close.png"
                fillMode:Image.PreserveAspectFit
            }
            onClicked: timeNomal.closeBtnClick()
        }
    }

    StackLayout{
        id: swipeView
        anchors.top: parent.top
        anchors.topMargin: Destiny.dp(64)
        width: parent.width
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Destiny.dp(104)
        currentIndex: tabBar.currentIndex
        property int rowItemWidth: Destiny.dp(84)

        Rectangle{
            id:anticlockwise
            width: parent.width
            height: parent.height
            opacity: 1.0
            Row{
                anchors.centerIn: parent
                height: Destiny.dp(260)
                spacing: Destiny.dp(10)
                NumberAdjustPanel {
                    id:minius1
                    width: swipeView.rowItemWidth
                    height: parent.height
                    maxNum: 9
                }
                NumberAdjustPanel {
                    id:minius2
                    width: swipeView.rowItemWidth
                    height: parent.height

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
                    font.family: String.font
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
            opacity: 0.0
            Row{
                anchors.centerIn: parent
                height: 260
                spacing: 10
                Text {
                    height: parent.height
                    font.pixelSize: Destiny.dp(120)
                    verticalAlignment:Text.AlignVCenter
                    text: qsTr("00")
                    color: "#2b2b2b"
                    font.bold: true
                    font.family: String.font
                    font.letterSpacing:Destiny.dp(14)

                }
                Text {
                    height: parent.height
                    font.pixelSize: Destiny.dp(120)
                    verticalAlignment:Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.baseline: parent.verticalCenter
                    anchors.baselineOffset: font.pixelSize/3.5
                    text: qsTr(":")
                    color: "#2b2b2b"
                    font.bold: true
                    font.family: String.font
                }
				 Text {
                    height: parent.height
					width: Destiny.dp(4)
                }
                Text {
                    height: parent.height
                    font.pixelSize: Destiny.dp(120)
                    verticalAlignment:Text.AlignVCenter
                    text: qsTr("00")
                    color: "#2b2b2b"
                    font.bold: true
                    font.family: String.font
                    font.letterSpacing:Destiny.dp(14)
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

    CustomButton {
        id:startTimerBtn
        font.pixelSize: Destiny.dp(18)
        height: Destiny.dp(64)
        width: Destiny.dp(256)
        anchors.horizontalCenter:   parent.horizontalCenter
        text: "开始计时"
        anchors.bottomMargin:  Destiny.dp(40)
        anchors.bottom: parent.bottom
        fontColor: "white"
        font.bold: true
        radius: height/2
        backgroudColor:"#008FFF"
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


