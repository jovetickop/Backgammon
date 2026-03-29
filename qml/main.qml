import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

// 主界面
Rectangle {
    id: root
    width: 1600
    height: 1200
    visible: true

    // 渐变背景
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#1a1a2e" }
        GradientStop { position: 0.5; color: "#16213e" }
        GradientStop { position: 1.0; color: "#0f3460" }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // ===== 左侧控制面板 =====
        Rectangle {
            id: leftPanel
            Layout.preferredWidth: 380
            Layout.fillHeight: true
            radius: 20
            color: "#2a2a4a"
            opacity: 0.85

            // 玻璃拟态阴影
            layer.effect: DropShadow {
                color: "#000000"
                radius: 20
                samples: 16
                spread: 0.3
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 16

                // 标题
                Text {
                    text: "🤖 五子棋 AI"
                    font.pixelSize: 32
                    font.bold: true
                    color: "#ffffff"
                    Layout.alignment: Qt.AlignHCenter
                }

                // 开始/重置按钮
                StyledButton {
                    id: startButton
                    text: "开始对局"
                    primary: true
                    onClicked: gameBridge.startGame()
                }

                // 历史对局按钮
                StyledButton {
                    text: "历史对局"
                    secondary: true
                    onClicked: gameBridge.showHistory()
                }

                // AI 说明按钮
                StyledButton {
                    text: "AI 说明"
                    secondary: true
                    onClicked: gameBridge.showAiInfo()
                }

                // 分隔线
                HorizontalDivider { }

                // 对局设置
                Text {
                    text: "对局设置"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#e0e0e0"
                }

                // 先手选择
                ComboBox {
                    id: starterCombo
                    width: parent.width
                    model: ["我先手", "AI 先手"]
                    currentIndex: 0
                    onCurrentIndexChanged: gameBridge.setPlayerStarts(currentIndex === 0)
                }

                // AI 难度
                Text {
                    text: "AI 难度"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#e0e0e0"
                }

                ComboBox {
                    id: difficultyCombo
                    width: parent.width
                    model: ["简单 (2步)", "中等 (3步)", "困难 (4步)", "大师 (5步)"]
                    currentIndex: 1
                    onCurrentIndexChanged: gameBridge.setDifficulty(currentIndex + 2)
                }

                // 分隔线
                HorizontalDivider { }

                // 统计信息
                Text {
                    text: "历史战绩"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#e0e0e0"
                }

                Text {
                    id: totalGamesLabel
                    text: "累计局数: 0"
                    color: "#b0b0b0"
                }

                Text {
                    id: playerRateLabel
                    text: "我: 0 胜 | 胜率 0%"
                    color: "#4ecdc4"
                }

                Text {
                    id: aiRateLabel
                    text: "AI: 0 胜 | 胜率 0%"
                    color: "#ff6b6b"
                }

                // 分隔线
                HorizontalDivider { }

                // 当前局面
                Text {
                    text: "当前局面"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#e0e0e0"
                }

                Text {
                    id: moveCountLabel
                    text: "当前手数: 0"
                    color: "#b0b0b0"
                }

                Text {
                    id: playerWinRateLabel
                    text: "我的预估胜率: 50%"
                    color: "#4ecdc4"
                }

                Text {
                    id: aiWinRateLabel
                    text: "AI 预估胜率: 50%"
                    color: "#ff6b6b"
                }

                // 胜率走势图
                Rectangle {
                    id: chartContainer
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    radius: 12
                    color: "#1a1a3a"

                    Canvas {
                        id: winRateCanvas
                        anchors.fill: parent
                        anchors.margins: 10
                        onPaint: gameBridge.paintWinRateChart(context2D, width, height)
                    }
                }

                // AI 思考切换
                RowLayout {
                    Text {
                        text: "显示 AI 思考"
                        color: "#b0b0b0"
                    }
                    Switch {
                        id: thinkSwitch
                        onCheckedChanged: gameBridge.setShowTop10(checked)
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // ===== 右侧棋盘 =====
        Rectangle {
            id: rightPanel
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 20
            color: "#2a2a4a"
            opacity: 0.85

            layer.effect: DropShadow {
                color: "#000000"
                radius: 20
                samples: 16
                spread: 0.3
            }

            // 棋盘
            BoardView {
                id: boardView
                anchors.centerIn: parent
                boardSize: 720
                cellSize: boardSize / 15
                onCellClicked: gameBridge.handlePlayerMove(row, col)
            }
        }
    }

    // 游戏状态变化时更新界面
    Connections {
        target: gameBridge

        function onGameStarted() {
            startButton.text = "重置对局"
            boardView.clearBoard()
        }

        function onGameReset() {
            startButton.text = "开始对局"
            boardView.clearBoard()
        }

        function onStatsUpdated(totalGames, playerWins, aiWins, playerRate, aiRate) {
            totalGamesLabel.text = "累计局数: " + totalGames
            playerRateLabel.text = `我: ${playerWins} 胜 | 胜率 ${playerRate}%`
            aiRateLabel.text = `AI: ${aiWins} 胜 | 胜率 ${aiRate}%`
        }

        function onMovePlaced(row, col, isPlayer) {
            boardView.placePiece(row, col, isPlayer)
        }

        function onMoveCountChanged(count) {
            moveCountLabel.text = "当前手数: " + count
        }

        function onWinRateUpdated(playerRate, aiRate) {
            playerWinRateLabel.text = `我的预估胜率: ${playerRate}%`
            aiWinRateLabel.text = `AI 预估胜率: ${aiRate}%`
            winRateCanvas.requestPaint()
        }

        function onGameOver(winner) {
            boardView.highlightWin(winner)
            resultDialog.visible = true
        }
    }

    // 结果对话框
    ResultDialog {
        id: resultDialog
        visible: false
    }
}

// ===== 样式按钮组件 =====
StyledButton {
    id: button1
    text: "Button"
}

// ===== 辅助组件 =====
Rectangle {
    id: horizontalDivider
    height: 1
    color: "#3a3a5a"
}
