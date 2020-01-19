import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
Rectangle {
    id:timeRuningItem
    radius: 10;
    color: timer.running?"#2482CB":"#38B597"
    anchors.centerIn: parent
    property int totalTime: 0
    property bool  positiveTime: true
    height: parent.height-10
    width: parent.width-10
    signal closeClick();
    signal backBtnClick();
    signal entryMinizeTimeState()
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
            PropertyChanges {target: timeRuningItem.parent;width:256;height:80}
            PropertyChanges {target: startTimerBtn;visible:false}
            PropertyChanges {target: closeBtn;visible:false }
            PropertyChanges {target: showTimeText;visible:false }
        },
        State {
            name:"timeout"
            PropertyChanges {target: startTimerBtn;text:"知道了";visible:true}
            PropertyChanges {target: showTimeText;text:"时间到";}
            PropertyChanges {target: canvas;visible:false;}
        }
    ]

    Canvas{  // 绘制计时器进度
        id : canvas
        width: timeRuningItem.width/2
        height: timeRuningItem.height/2
        anchors.centerIn: timeRuningItem
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
            for(var i = angle;i<=90;i++){
                ctx.rotate(-4* Math.PI / 180)
                ctx.moveTo(0, -canvas.height/2+10)
                ctx.lineTo(0, -canvas.height/2)
                ctx.strokeStyle = 'rgba(255, 255, 255, 1)'
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
        font.pixelSize: 18
        height: 64
        width: parent.width/2
        anchors.horizontalCenter:   parent.horizontalCenter
        anchors.bottomMargin:  40
        anchors.bottom: parent.bottom
        fontColor: "white"
        radius: 32
        backgroudColor:"transparent"
        borderColor: "white"
        borderWidth: 3
        onClicked: {
            if(timeRuningItem.state == "timeout"||timeRuningItem.state == "stopTime"){
                timeRuningItem.backBtnClick();
            }else if(timeRuningItem.state == "runingTime"){
                timeRuningItem.state = "stopTime"
            }
        }
    }

    Text { // 展示文字
        id:showTimeText
        font.pixelSize:90
        anchors.centerIn: parent
        color: "white"
        text: transformDate(totalTime)
    }

    Button { // 关闭按钮
        id:closeBtn
        font.pixelSize: 10
        anchors.right: parent.right
        anchors.top:parent.top
        width: 40
        height: 40
        anchors.rightMargin :  20
        anchors.topMargin :  20
        background: Image{
            source: "./close_white.png"
            fillMode:Image.PreserveAspectFit
        }
        onClicked: {
            timeRuningItem.closeClick();}
    }

    Row{
        id:minizeShowContent
        visible: false
        anchors.centerIn: parent
        spacing: 16
        CustomButton {
            font.pixelSize: 12
            width: parent.width/4
            height: parent.height/2
            anchors.verticalCenter: minizeShowContent.verticalCenter
            fontColor: "white"
            radius: 15
            backgroudColor:"transparent"
            borderColor: "white"
            borderWidth: 1
            text:"停止"
            onClicked: {
                timeRuningItem.state= "stopTime"
                canvas.requestPaint()
            }
        }
        Text {
            font.pixelSize:60
            color: "white"
            text: showTimeText.text
        }
    }

    function transformDate(total){
        var second = total%60;
        var minus = parseInt(total/60);
        return (minus > 9 ? minus : ("0" + minus)) +":" + (second > 9 ? second : ("0" + second))
    }

}

