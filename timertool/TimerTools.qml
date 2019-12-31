import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12

Rectangle {
    id:rootItem
    width: 558
    height: 566
    visible: true
    color: "#00000000"
    property var  timeRunningView: undefined
    signal timerClose()
    Image{
        anchors.fill: parent
        source: "bg_icon.png"
    }

    TimerNormal{
        height: parent.height-40
        width: parent.width-40
        radius: 10;
        anchors.centerIn: parent
        id:normal
        onRuningClick:{
            timeRunningView = Qt.createComponent("TimerRuning.qml").createObject(rootItem,
                                                                                {
                                                                                    totalTime:normal.totalTime,
                                                                                    positiveTime:totalTime<=0})
            visible = false;
            timeRunningView.backBtnClick.connect(timeRuningViewClose)
            timeRunningView.closeClick.connect(rootItem.timerClose)
        }
        function timeRuningViewClose(){
            timeRunningView.destroy();
            normal.visible = true;
        }
        onCloseBtnClick:{
            rootItem.timerClose()
        }

    }
}
