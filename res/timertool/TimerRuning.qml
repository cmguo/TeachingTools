import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import Strings 1.0
import TalDisplay 1.0

Rectangle {
    id:timeRuningItem
    radius: Destiny.dp(8);
    anchors.centerIn: parent
    property int totalTime: 0
    property bool  positiveTime: true
    height: parent.height
    width: parent.width
    signal closeClick();
    signal backBtnClick();
    signal entryMinizeTimeState()

    Image {
        id: bg
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        source: timer.running ? "./timer_bg_2.png":"./timer_bg_1.png"
    }

    states: [
        State {
            name: "runingTime"
            PropertyChanges {target: timer;running:true;}
            PropertyChanges {target: startTimerBtn;text:"停止计时";visible:true}
            PropertyChanges {target: closeBtn;visible:true}
            PropertyChanges {target: timer;intervalTime:0}
        },
        State {
            name: "stopTime"
            PropertyChanges {target: startTimerBtn;text:"返回"}
        },
        State {
            name: "minizeTime"
            PropertyChanges {target: timer;running:true;}
            PropertyChanges {target: canvas;visible:false }
            PropertyChanges {target: minizeShowContent;visible:true}
            PropertyChanges {target: timeRuningItem.parent;width:Destiny.dp(356);height:Destiny.dp(80)}
            PropertyChanges {target: startTimerBtn;visible:false}
            PropertyChanges {target: closeBtn;visible:false }
            PropertyChanges {target: showTimeText;visible:false }
        },
        State {
            name:"timeout"
            PropertyChanges {target: startTimerBtn;text:"知道了";visible:true}
            PropertyChanges {target: showTimeText;text:"时间到";}
            PropertyChanges {target: canvas;visible:true;}
        }
    ]


    Item {
        anchors.top: parent.top
        anchors.topMargin: Destiny.dp(64)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Destiny.dp(104)
        width: parent.width
        height: parent.height - anchors.topMargin - anchors.bottomMargin


        Canvas{  // 绘制计时器进度
            id : canvas
            width: Destiny.dp(264+18)
            height: Destiny.dp(264+18)
            anchors.centerIn: parent
            visible: !positiveTime
            property int angle: 0
            onPaint: {
                if(timeRuningItem.state == "minizeTime")
                    return;
                var ctx = getContext("2d");
                ctx.clearRect(0,0,width,height)
                ctx.save()
                ctx.lineWidth=2;
                ctx.translate(canvas.width/2,canvas.height/2)
                ctx.beginPath();
                for(var i = 1;i<angle;i++){
                    ctx.rotate(-4* Math.PI / 180)
                    ctx.moveTo(0, -canvas.height/2+10)
                    ctx.lineTo(0, -canvas.height/2)
                    ctx.strokeStyle = 'rgba(255, 255, 255, 0.05)'
                }
                ctx.stroke()
                ctx.beginPath();
                for(var i = angle;i<90;i++){
                    ctx.rotate(-4* Math.PI / 180)
                    ctx.moveTo(0, -canvas.height/2+10)
                    ctx.lineTo(0, -canvas.height/2)
                    ctx.strokeStyle = 'rgba(255, 255, 255, 0.5)'
                }
                ctx.stroke()
                ctx.restore()
            }

            Component.onCompleted: {
                timeRuningItem.state = "runingTime"
                timer.running = true;
                timer.intervalTotalTime = timeRuningItem.totalTime;

            }
            Timer{
                id:timer
                interval: 1000
                repeat: true
                triggeredOnStart: true
                property int intervalTime: 0 ;
                property int intervalTotalTime: 0
                onTriggered: {
                    timeRuningItem.totalTime = positiveTime?timeRuningItem.totalTime+1:timeRuningItem.totalTime-1
                    showTimeText.text = transformDate(timeRuningItem.totalTime)
                    intervalTime++;

                    if(intervalTime == 5 && timeRuningItem.state == "runingTime"){
                        timeRuningItem.state = "minizeTime"
                        intervalTime =0;
                        timeRuningItem.entryMinizeTimeState()
                    }

                    if(!positiveTime){
                        canvas.angle = 90-timeRuningItem.totalTime/intervalTotalTime*90
                        canvas.requestPaint()
                        if(totalTime<=0)
                            timeRuningItem.state  = "timeout"
                    }
                }
            }
        }

        Text { // 展示文字
            id:showTimeText
            font.pixelSize: Destiny.sp(120)
            anchors.centerIn: parent
            font.family: String.font
            font.bold: true
            color: "white"
            text: transformDate(totalTime)
             font.letterSpacing:Destiny.dp(14)
        }

    }

    MouseArea { // 注册全局可点击，最小化时使用
        id :mouseArea
        anchors.fill: parent
        property var prex: 0
        property var prey: 0
        onPressed: { //
            prex= mapToGlobal(mouse.x,mouse.y).x
            prey=mapToGlobal(mouse.x,mouse.y).y
        }
        onReleased: { //区分点击，如果是拖动的话，有加载的quickwidget响应拖动事件
            var isDrag = Math.abs(mapToGlobal(mouse.x,mouse.y).x-prex)>5||Math.abs(mapToGlobal(mouse.x,mouse.y).y-prey)>5;
            if(!isDrag &&timeRuningItem.state == "minizeTime"){
                timeRuningItem.state= "runingTime"
            }
        }
    }


    CustomButton { // 底部点击btn
        id:startTimerBtn
        font.pixelSize: Destiny.sp(18)
        height: Destiny.dp(64)
        width: Destiny.dp(256)
        anchors.horizontalCenter:   parent.horizontalCenter
        anchors.bottomMargin:  Destiny.dp(40)
        anchors.bottom: parent.bottom
        fontColor: "white"
        font.bold: true
        radius: height/2
        backgroudColor:"transparent"
        borderColor: "white"
        borderWidth: Destiny.dp(3)
        onClicked: {
            if(timeRuningItem.state == "timeout"||timeRuningItem.state == "stopTime"){
                timeRuningItem.backBtnClick();
            }else if(timeRuningItem.state == "runingTime"){
                timeRuningItem.state = "stopTime"
            }
        }
    }


    Button { // 关闭按钮
        id:closeBtn
        anchors.right: parent.right
        anchors.top:parent.top
        width: Destiny.dp(40)
        height: Destiny.dp(40)
        anchors.rightMargin :  Destiny.dp(24)
        anchors.topMargin :  Destiny.dp(12)
        background: Image{
            source: "./close_white.png"
            fillMode:Image.PreserveAspectFit
        }
        onClicked: {
            timeRuningItem.closeClick();
        }
    }

    Rectangle{
        id:minizeShowContent
        visible: false
        color: "transparent"
        anchors.fill: parent
        CustomButton {
            id:stopButton
            anchors.left: parent.left
            anchors.leftMargin: Destiny.dp(24)
            font.pixelSize: Destiny.sp(16)
            width: Destiny.dp(83)
            height: Destiny.dp(40)
            anchors.verticalCenter: minizeShowContent.verticalCenter
            fontColor: "white"
            radius: height/2
            backgroudColor:"transparent"
            borderColor: "white"
            borderWidth: Destiny.dp(2)
            text:"停止"
            onClicked: {
                timeRuningItem.state= "stopTime"
                canvas.requestPaint()
            }
        }


        Text {
            font.pixelSize: Destiny.sp(60)
            anchors.left: stopButton.right
            anchors.right: parent.right
            width: Destiny.dp(209)
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            color: "white"
            font.bold: true
            font.family: String.font
            text: showTimeText.text
            font.letterSpacing:Destiny.dp(8)
        }
    }

    function transformDate(total){
        var second = total%60;
        var minus = parseInt(total/60);
        return (minus > 9 ? minus : ("0" + minus)) +":" + (second > 9 ? second : ("0" + second))
    }

}

