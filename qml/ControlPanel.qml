import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

/**
 * ControlPanel - 左侧控制面板组件
 * 包含游戏控制、设置、统计信息
 */
Rectangle {
    id: root
    width: 360
    Layout.fillHeight: true

    // 面板背景
    radius: 24
    color: "#1e1e2e"
    opacity: 0.95
    border.color: "#ffffff"
    border.width: 1
    opacity: 0.1

    layer.effect: DropShadow {
        color: "#000000"
        radius: 30
        samples: 20
        spread: 0.5
    }

    // 内部布局
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        // --- 标题 ---
        Text {
            text: "⚫ ⚪ 五子棋 AI"
            font.pixelSize: 28
            font.bold: true
            color: "#ffffff"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // --- 开始按钮 ---
        Button {
            id: startBtn
            text: "开始对局"
            width: parent.width
            height: 56
            font.pixelSize: 18
            font.bold: true

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

            onClicked: gameBridge.startGame()

            scale: pressed ? 0.95 : 1.0
            Behavior on scale { NumberAnimation { duration: 100 } }
        }

        // --- 功能按钮组 ---
        Row {
            width: parent.width
            height: 48
            spacing: 12

            Button {
                width: parent.width / 2 - 6
                height: parent.height
                text: "📜 历史"
                font.pixelSize: 14
                background: Rectangle {
                    radius: 8
                    color: "#2d2d44"
                    border.color: "#3d3d5c"
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    color: "#b0b0c0"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                }
                onClicked: gameBridge.showHistory()
            }

            Button {
                width: parent.width / 2 - 6
                height: parent.height
                text: "🤖 AI"
                font.pixelSize: 14
                background: Rectangle {
                    radius: 8
                    color: "#2d2d44"
                    border.color: "#3d3d5c"
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    color: "#b0b0c0"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                }
                onClicked: gameBridge.showAiInfo()
            }
        }

        // --- 分隔线 ---
        Divider {}

        // --- 对局设置 ---
        Text {
            text: "⚙️ 对局设置"
            font.pixelSize: 16
            font.bold: true
            color: "#e0e0e0"
        }

        // 先手选择
        Text {
            text: "谁先手"
            color: "#8888aa"
            font.pixelSize: 12
        }

        ComboBox {
            id: starterCombo
            width: parent.width
            model: ["我先手", "AI 先手"]
            currentIndex: 0
            background: Rectangle {
                radius: 8
                color: "#2d2d44"
            }
            onCurrentIndexChanged: gameBridge.setPlayerStarts(currentIndex === 0)
        }

        // AI 难度
        Text {
            text: "AI 难度"
            color: "#8888aa"
            font.pixelSize: 12
        }

        ComboBox {
            id: difficultyCombo
            width: parent.width
            model: ["简单 (2步)", "中等 (3步)", "困难 (4步)", "大师 (5步)"]
            currentIndex: 1
            background: Rectangle {
                radius: 8
                color: "#2d2d44"
            }
            onCurrentIndexChanged: gameBridge.setDifficulty(currentIndex + 2)
        }

        // --- 分隔线 ---
        Divider {}

        // --- 统计信息 ---
        Text {
            text: "📊 历史战绩"
            font.pixelSize: 16
            font.bold: true
            color: "#e0e0e0"
        }

        Column {
            width: parent.width
            spacing: 8

            Text {
                id: totalGames
                text: "总局数: 0"
                color: "#8888aa"
                font.pixelSize: 13
            }
            Text {
                id: playerWins
                text: "我: 0 胜 (0%)"
                color: "#4ecdc4"
                font.pixelSize: 13
            }
            Text {
                id: aiWins
                text: "AI: 0 胜 (0%)"
                color: "#ff6b6b"
                font.pixelSize: 13
            }
        }

        // --- 分隔线 ---
        Divider {}

        // --- 当前局面 ---
        Text {
            text: "🎯 当前局面"
            font.pixelSize: 16
            font.bold: true
            color: "#e0e0e0"
        }

        Column {
            width: parent.width
            spacing: 8

            Text {
                id: moveCount
                text: "手数: 0"
                color: "#8888aa"
                font.pixelSize: 13
            }
            Text {
                id: playerRate
                text: "我胜率: 50%"
                color: "#4ecdc4"
                font.pixelSize: 15
            }
            Text {
                id: aiRate
                text: "AI胜率: 50%"
                color: "#ff6b6b"
                font.pixelSize: 15
            }
        }

        // --- 胜率图 ---
        WinRateChart {
            id: chartComponent
            width: parent.width
            height: 160
        }

        // --- AI 思考开关 ---
        Row {
            width: parent.width
            height: 40

            Text {
                text: "💭 显示 AI 思考"
                color: "#b0b0c0"
                font.pixelSize: 14
                verticalAlignment: Vertical.Center
            }

            Switch {
                id: thinkSwitch
                onCheckedChanged: gameBridge.setShowTop10(checked)
            }
        }

        Item { Layout.fillHeight: true }

        // --- 版权 ---
        Text {
            text: "Powered by Claude Code"
            color: "#555566"
            font.pixelSize: 10
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    // ---------------- 公共方法 ----------------

    // 更新统计信息
    function updateStats(total, playerWin, aiWin, pRate, aRate) {
        totalGames.text = "总局数: " + total
        playerWins.text = `我: ${playerWin} 胜 (${pRate}%)`
        aiWins.text = `AI: ${aiWin} 胜 (${aRate}%)`
    }

    // 更新局面信息
    function updateGameState(count, pRate, aRate) {
        moveCount.text = "手数: " + count
        playerRate.text = "我胜率: " + pRate + "%"
        aiRate.text = "AI胜率: " + aRate + "%"
        chartComponent.requestPaint()
    }

    // 更新开始按钮文本
    function setStarted(isStarted) {
        startBtn.text = isStarted ? "重新开始" : "开始对局"
    }
}

// 分隔线组件
Rectangle {
    id: divider
    width: parent.width
    height: 1
    color: "#3d3d5c"
}
