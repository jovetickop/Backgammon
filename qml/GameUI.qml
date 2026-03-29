import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/**
 * GameUI - 五子棋 AI 主界面
 * 组合 BoardView、ControlPanel、ResultDialog 等组件
 */
Rectangle {
    id: root
    width: 1600
    height: 1200

    // 渐变背景
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#0f0c29" }
        GradientStop { position: 0.5; color: "#302b63" }
        GradientStop { position: 1.0; color: "#24243e" }
    }

    // 主布局
    RowLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 24

        // 左侧控制面板
        ControlPanel {
            id: controlPanel
            Layout.preferredWidth: 360
            Layout.fillHeight: true
        }

        // 右侧棋盘
        Rectangle {
            id: boardContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 24
            color: "#1e1e2e"
            opacity: 0.95

            layer.effect: DropShadow {
                color: "#000000"
                radius: 30
                samples: 20
                spread: 0.5
            }

            // 棋盘视图
            BoardView {
                id: boardView
                anchors.centerIn: parent
                boardSize: Math.min(boardContainer.width - 40, boardContainer.height - 40)

                onCellClicked: function(row, col) {
                    gameBridge.handlePlayerMove(row, col)
                }
            }
        }
    }

    // 结果对话框
    ResultDialog {
        id: resultDialog
        anchors.fill: parent
    }

    // ---------------- 信号连接 ----------------
    Connections {
        target: gameBridge

        // 游戏开始
        function onGameStarted() {
            controlPanel.setStarted(true)
            boardView.clearBoard()
        }

        // 统计更新
        function onStatsUpdated(total, playerWin, aiWin, pRate, aRate) {
            controlPanel.updateStats(total, playerWin, aiWin, pRate, aRate)
        }

        // 棋子放置
        function onMovePlaced(row, col, isPlayer) {
            boardView.placePiece(row, col, isPlayer)
        }

        // 手数更新
        function onMoveCountChanged(count) {
            // 获取当前胜率并更新
            var pRate = gameBridge.getPlayerWinRate()
            var aRate = gameBridge.getAiWinRate()
            controlPanel.updateGameState(count, pRate, aRate)
        }

        // 胜率更新
        function onWinRateUpdated(pRate, aRate) {
            controlPanel.updateGameState(
                gameBridge.getMoveCount(),
                pRate,
                aRate
            )
        }

        // 游戏结束
        function onGameOver(winner, positions) {
            boardView.setWinLine(positions)
            controlPanel.setStarted(false)
            resultDialog.open(winner)
        }
    }
}
