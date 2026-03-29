import QtQuick 2.15
import QtGraphicalEffects 1.15

/**
 * BoardView - 棋盘组件
 * 负责棋盘绘制、棋子渲染、点击交互
 */
Rectangle {
    id: root

    // 棋盘尺寸
    property int boardSize: 720
    property int gridSize: 15
    property int cellSize: boardSize / gridSize

    // 悬停状态
    property int hoverRow: -1
    property int hoverCol: -1

    // 信号
    signal cellClicked(int row, int col)

    // 棋子数据模型
    ListModel { id: piecesModel }

    // 高亮位置列表
    ListModel { id: highlightModel }

    // 木纹背景
    width: boardSize
    height: boardSize
    radius: 8
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#d4a76a" }
        GradientStop { position: 0.5; color: "#c8a06c" }
        GradientStop { position: 1.0; color: "#b8895a" }
    }

    // 阴影效果
    layer.effect: DropShadow {
        color: "#000000"
        radius: 20
        samples: 25
        spread: 0.3
    }

    // ---------------- 网格层 ----------------
    Item {
        id: gridLayer
        anchors.fill: parent
        anchors.margins: cellSize / 2

        // 垂直线
        Repeater {
            model: gridSize
            Rectangle {
                x: index * cellSize
                width: 1.5
                height: parent.height
                color: "#5a4030"
            }
        }

        // 水平线
        Repeater {
            model: gridSize
            Rectangle {
                y: index * cellSize
                width: parent.width
                height: 1.5
                color: "#5a4030"
            }
        }
    }

    // ---------------- 星位 ----------------
    Column {
        anchors.fill: parent
        anchors.margins: cellSize / 2
        spacing: cellSize * 4
        Repeater {
            model: 3
            Row {
                spacing: cellSize * 4
                Rectangle {
                    width: 10; height: 10; radius: 5; color: "#3a2820"
                }
                Rectangle {
                    width: 10; height: 10; radius: 5; color: "#3a2820"
                }
            }
        }
    }

    // ---------------- 悬停高亮 ----------------
    Rectangle {
        id: hoverRect
        width: cellSize * 0.85
        height: cellSize * 0.85
        radius: width / 2
        color: "#ffffff"
        opacity: 0.22
        visible: hoverRow >= 0 && hoverCol >= 0
        x: hoverCol * cellSize + cellSize / 2 - width / 2
        y: hoverRow * cellSize + cellSize / 2 - height / 2

        Behavior on x { SmoothedAnimation { duration: 50 } }
        Behavior on y { SmoothedAnimation { duration: 50 } }
    }

    // ---------------- 棋子层 ----------------
    Repeater {
        model: piecesModel
        delegate: pieceDelegate
    }

    // 棋子组件
    Component {
        id: pieceDelegate

        Rectangle {
            property int pieceSize: cellSize * 0.84
            x: col * cellSize + cellSize / 2 - pieceSize / 2
            y: row * cellSize + cellSize / 2 - pieceSize / 2
            width: pieceSize
            height: pieceSize
            radius: pieceSize / 2

            // 棋子渐变
            gradient: Gradient {
                GradientStop { position: 0.0; color: isBlack ? "#5a5a5a" : "#f8f8f8" }
                GradientStop { position: 0.5; color: isBlack ? "#2a2a2a" : "#ffffff" }
                GradientStop { position: 1.0; color: isBlack ? "#1a1a1a" : "#d0d0d0" }
            }

            // 最后一步标记
            Rectangle {
                width: pieceSize * 0.3
                height: width
                radius: width / 2
                color: isBlack ? "#ffffff" : "#000000"
                anchors.centerIn: parent
                opacity: 0.75
            }

            // 落下动画
            scale: 0
            NumberAnimation on scale {
                to: 1
                duration: 150
                easing.type: Easing.OutBack
            }

            // 阴影
            layer.effect: DropShadow {
                color: "#000000"
                radius: 4
                samples: 8
                spread: 0.2
            }
        }
    }

    // ---------------- 获胜连线 ----------------
    Canvas {
        id: winLineCanvas
        anchors.fill: parent
        onPaint: {
            if (highlightModel.count < 2) return
            var ctx = getContext("2d")
            ctx.lineWidth = 5
            ctx.lineCap = "round"
            ctx.strokeStyle = "#ff4444"

            var first = highlightModel.get(0)
            var last = highlightModel.get(highlightModel.count - 1)

            ctx.beginPath()
            ctx.moveTo(first.col * cellSize + cellSize / 2,
                       first.row * cellSize + cellSize / 2)
            ctx.lineTo(last.col * cellSize + cellSize / 2,
                       last.row * cellSize + cellSize / 2)
            ctx.stroke()
        }
    }

    // ---------------- 鼠标交互 ----------------
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onMouseXChanged: updateHover()
        onMouseYChanged: updateHover()

        function updateHover() {
            var gx = Math.floor((mouseX - cellSize / 2) / cellSize)
            var gy = Math.floor((mouseY - cellSize / 2) / cellSize)

            if (gx >= 0 && gx < gridSize && gy >= 0 && gy < gridSize) {
                hoverRow = gy
                hoverCol = gx
            } else {
                hoverRow = -1
                hoverCol = -1
            }
        }

        onClicked: {
            if (hoverRow >= 0 && hoverCol >= 0) {
                cellClicked(hoverRow, hoverCol)
            }
        }
    }

    // ---------------- 公共方法 ----------------

    // 放置棋子
    function placePiece(row, col, isPlayer) {
        piecesModel.append({
            "row": row,
            "col": col,
            "isBlack": isPlayer
        })
    }

    // 清空棋盘
    function clearBoard() {
        piecesModel.clear()
        highlightModel.clear()
        winLineCanvas.requestPaint()
    }

    // 设置获胜连线
    function setWinLine(positions) {
        highlightModel.clear()
        for (var i = 0; i < positions.length; i++) {
            highlightModel.append(positions[i])
        }
        winLineCanvas.requestPaint()
    }
}
