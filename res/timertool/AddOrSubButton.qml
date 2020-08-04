import QtQuick 2.0
import QtQuick.Controls 2.5
import TalDisplay 1.0

import "qrc:/uibase/qml/talwidget"

Item {
    property int buttonSize: Destiny.dp(32)
    property bool isAdd: true

    property bool isPressed: false
    property bool isLongPressed: false

    signal click()

    id: addOrSubRoot

    Image {
        id: btnImage
        width: buttonSize
        height: buttonSize
        clip: false
        anchors.centerIn: parent

        source: {
            if (!addOrSubRoot.enabled) {
                return getResource(2)
            } else if (isPressed) {
                return getResource(1)
            } else {
                return getResource(0)
            }
        }

        TalMouseArea {
            id: mouseArea
            anchors.fill: parent
            onStateChanged: {
                var current = state === 2
                if (isPressed !== current) {
                    isPressed = current
                }
            }

            onClicked: {
                addOrSubRoot.click()
            }
        }
    }

    // type: 0 normal 1 pressed 2 disable
    function getResource(type) {
        var base = "qrc:/teachingtools/timertool/" + (isAdd ? "add_" : "sub_")
        if (type === 1) {
            return base + "pressed.png"
        } else if (type === 2) {
            return base + "disabled.png"
        } else {
            return base + "normal.png"
        }
    }
}
