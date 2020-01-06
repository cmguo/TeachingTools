import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12


Rectangle{
    id:timeNomal
    property int totalTime: 0
    signal runingClick();
    signal closeBtnClick();
    Rectangle{
        id:topRect
        width: parent.width
        border.width: 1
        border.color: "white"
        radius: parent.radius+border.width

        gradient: Gradient {
            GradientStop{ position: 0.0; color: "#F9F9F9";}
            GradientStop{ position: 1.0; color: "#ECECEC";}
        }
        height: 80
        TabBar {
            height: 40
            font.pointSize: 14
            width: 194
            anchors.centerIn:  parent
            id: tabBar
            spacing: -8
            currentIndex: 1
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
        Rectangle{ // 遮挡底部圆角
            height: parent.radius-parent.border.width;
            width: parent.width
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.border.width
        }
    }


    StackLayout{
        id: swipeView
        anchors.top: topRect.bottom
        width: parent.width
        anchors.bottom: startTimerBtn.top
        currentIndex: tabBar.currentIndex
        property int rowItemWidth: 84
        Rectangle{
            id:anticlockwise
            width: parent.width
            height: parent.height
            opacity: 1.0
            Row{
                anchors.centerIn: parent
                height: 260
                spacing: 10
                NumberAdjustPanel {
                    id:minius1
                    width: swipeView.rowItemWidth
                    height: parent.height
                    maxNum: 5
                }
                NumberAdjustPanel {
                    id:minius2
                    width: swipeView.rowItemWidth
                    height: parent.height

                }
                Text {
                    height: parent.height
                    width: 40
                    font.pixelSize: 120
                    verticalAlignment:Text.AlignVCenter
                    anchors.baseline: parent.verticalCenter
                    anchors.baselineOffset: font.pixelSize/4
                    text: qsTr(":")
                }
                Text {  // 因为设置了spcing导致不是居中，此Item == 插入一个spacing
                    height: parent.height
                    width: 1
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
                    font.pixelSize: 120
                    verticalAlignment:Text.AlignVCenter
                    text: qsTr("00")
                }
                Text {
                    height: parent.height
                    font.pixelSize: 120
                    verticalAlignment:Text.AlignVCenter
                    anchors.baseline: parent.verticalCenter
                    anchors.baselineOffset: font.pixelSize/4
                    text: qsTr(":")
                }
                Text {
                    height: parent.height
                    font.pixelSize: 120
                    verticalAlignment:Text.AlignVCenter
                    text: qsTr("00")
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
        font.pixelSize: 18
        height: 64
        width: parent.width/2
        anchors.horizontalCenter:   parent.horizontalCenter
        text: "开始计时"
        anchors.bottomMargin:  40
        anchors.bottom: parent.bottom
        fontColor: "white"
        radius: 32
        backgroudColor:"#008FFF"
        onClicked: {
            setTotalTime();
            if(totalTime !=0||swipeView.currentIndex==1)
                timeNomal.runingClick();
        }
    }

    Button {
        font.pixelSize: 10
        anchors.right: parent.right
        anchors.top:parent.top
        width: 40
        height: 40
        anchors.rightMargin :  20
        anchors.topMargin :  20
        background: Image{
            source: "./close.png"
            fillMode:Image.PreserveAspectFit
        }
        onClicked: timeNomal.closeBtnClick()
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


