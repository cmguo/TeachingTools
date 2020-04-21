import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.12
import Strings 1.0
import TalDisplay 1.0
import "qrc:/qml/TalWidget"

Rectangle {
    id:rootItem
    width: Destiny.dp(528)
    height: Destiny.dp(508)
    visible: true
    color: "white"
    radius: Destiny.dp(8)
    property var  timeRunningView: undefined
    signal timerClose()
    signal entryMinizeTimeState()
    clip: true

    TimerNormal{
        id:normal
        width: parent.width
        height: parent.height
        clip: true
        radius: Destiny.dp(8)
        onRuningClick:{
            timeRunningView = Qt.createComponent("TimerRuning.qml").createObject(rootItem,
                                                                                {
                                                                                    totalTime:normal.totalTime,
                                                                                    positiveTime:totalTime<=0})
            visible = false;
            timeRunningView.backBtnClick.connect(timeRuningViewClose)
            timeRunningView.closeClick.connect(rootItem.timerClose)
            timeRunningView.entryMinizeTimeState.connect(rootItem.entryMinizeTimeState)
        }
        function timeRuningViewClose(){
            timeRunningView.visible = false;
            normal.visible = true;
            timeRunningView = undefined;
        }
        onCloseBtnClick:{
            rootItem.timerClose()
        }

    }
}
