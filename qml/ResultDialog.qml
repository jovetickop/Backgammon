import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

/**
 * ResultDialog - 游戏结果对话框
 */
Rectangle {
    id: root
    visible: false
    z: 100

    // 遮罩层
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.6
    }

    // 对话框主体
    Rectangle {
        width: 400
        height: 300
        radius: 24
        anchors.centerIn: parent

        layer.effect: DropShadow {
            radius: 40
            samples: 30
            spread: 0.5
        }

        gradient: Gradient {
            GradientStop { position: 0; color: "#2d2d44" }
            GradientStop { position: 1; color: "#1e1e2e" }
        }

        border.color: "#667eea"
        border.width: 2

        Column {
            anchors.centerIn: parent
            spacing: 24

            // 结果文字
            Text {
                id: resultText
                text: "你赢了！🎉"
                font.pixelSize: 36
                font.bold: true
                color: "#4ecdc4"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // 重新开始按钮
            Button {
                text: "再来一局"
                width: 200
                height: 52
                font.pixelSize: 16
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter

                background: Rectangle {
                    radius: 12
                    gradient: Gradient {
                        GradientStop { position: 0; color: "#667eea" }
                        GradientStop { position: 1; color: "#764ba2" }
                    }
                }

                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    close()
                    gameBridge.startGame()
                }

                scale: pressed ? 0.95 : 1.0
                Behavior on scale { NumberAnimation { duration: 100 } }
            }

            // 关闭按钮
            Button {
                text: "关闭"
                width: 200
                height: 40
                font.pixelSize: 14
                anchors.horizontalCenter: parent.horizontalCenter

                background: Rectangle {
                    radius: 8
                    color: "#2d2d44"
                    border.color: "#3d3d5c"
                    border.width: 1
                }

                contentItem: Text {
                    text: parent.text
                    color: "#8888aa"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                }

                onClicked: close()
            }
        }
    }

    // 打开对话框
    function open(winner) {
        if (winner === 1) {  // 玩家赢
            resultText.text = "你赢了！🎉"
            resultText.color = "#4ecdc4"
        } else {  // AI 赢
            resultText.text = "AI 赢了 🤖"
            resultText.color = "#ff6b6b"
        }
        visible = true

        // 弹出动画
        scale = 0.8
        NumberAnimation on scale {
            to: 1
            duration: 200
            easing.type: Easing.OutBack
        }
    }

    // 关闭对话框
    function close() {
        visible = false
    }
}
