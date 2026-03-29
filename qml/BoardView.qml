import QtQuick 2.15
import QtQuick.Shapes 1.15
import QtGraphicalEffects 1.15

// 棋盘视图组件
Rectangle {
    id: root
    width: boardSize
    height: boardSize
    radius: 8
    color: "#c8a06c" // 木纹色

    // 棋盘格子的行数和列数
    property int gridSize: 15
    property int boardSize: 720
    property int cellSize: boardSize / gridSize

    // 信号：点击棋盘格子
    signal cellClicked(int row, int col)

    // 鼠标悬停的格子
    property int hoverRow: -1
    property int hoverCol: -1

    // 存放棋子的模型
    ListModel {
        id: piecesModel
    }

    // 存放高亮位置（获胜连线）
    ListModel {
        id: highlightModel
    }

    // 背景渐变
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#d4a76a" }
        GradientStop { position: 0.5; color: "#c8a06c" }
        GradientStop { position: 1.0; color: "#b8895a" }
    }

    // 阴影效果
    layer.effect: DropShadow {
        color: "#000000"
        radius: 15
        samples: 20
        spread: 0.2
    }

    // 棋盘网格层
    Item {
        id: gridLayer
        anchors.fill: parent
        anchors.margins: cellSize / 2

        // 绘制网格线
        Shape {
            id: gridShape
            anchors.fill: parent

            ShapePath {
                id: gridPath
                strokeColor: "#5a4030"
                strokeWidth: 1.5
                fillColor: "transparent"

                // 垂直线
                PathMove { x: 0; y: 0 }
                PathLine { x: 0; y: parent.height }
                PathMove { x: cellSize; y: 0 }
                PathLine { x: cellSize; y: parent.height }
                PathMove { x: cellSize * 2; y: 0 }
                PathLine { x: cellSize * 2; y: parent.height }
                PathMove { x: cellSize * 3; y: 0 }
                PathLine { x: cellSize * 3; y: parent.height }
                PathMove { x: cellSize * 4; y: 0 }
                PathLine { x: cellSize * 4; y: parent.height }
                PathMove { x: cellSize * 5; y: 0 }
                PathLine { x: cellSize * 5; y: parent.height }
                PathMove { x: cellSize * 6; y: 0 }
                PathLine { x: cellSize * 6; y: parent.height }
                PathMove { x: cellSize * 7; y: 0 }
                PathLine { x: cellSize * 7; y: parent.height }
                PathMove { x: cellSize * 8; y: 0 }
                PathLine { x: cellSize * 8; y: parent.height }
                PathMove { x: cellSize * 9; y: 0 }
                PathLine { x: cellSize * 9; y: parent.height }
                PathMove { x: cellSize * 10; y: 0 }
                PathLine { x: cellSize * 10; y: parent.height }
                PathMove { x: cellSize * 11; y: 0 }
                PathLine { x: cellSize * 11; y: parent.height }
                PathMove { x: cellSize * 12; y: 0 }
                PathLine { x: cellSize * 12; y: parent.height }
                PathMove { x: cellSize * 13; y: 0 }
                PathLine { x: cellSize * 13; y: parent.height }
                PathMove { x: cellSize * 14; y: 0 }
                PathLine { x: cellSize * 14; y: parent.height }

                // 水平线
                PathMove { x: 0; y: 0 }
                PathLine { x: parent.width; y: 0 }
                PathMove { x: 0; y: cellSize }
                PathLine { x: parent.width; y: cellSize }
                PathMove { x: 0; y: cellSize * 2 }
                PathLine { x: parent.width; y: cellSize * 2 }
                PathMove { x: 0; y: cellSize * 3 }
                PathLine { x: parent.width; y: cellSize * 3 }
                PathMove { x: 0; y: cellSize * 4 }
                PathLine { x: parent.width; y: cellSize * 4 }
                PathMove { x: 0; y: cellSize * 5 }
                PathLine { x: parent.width; y: cellSize * 5 }
                PathMove { x: 0; y: cellSize * 6 }
                PathLine { x: parent.width; y: cellSize * 6 }
                PathMove { x: 0; y: cellSize * 7 }
                PathLine { x: parent.width; y: cellSize * 7 }
                PathMove { x: 0; y: cellSize * 8 }
                PathLine { x: parent.width; y: cellSize * 8 }
                PathMove { x: 0; y: cellSize * 9 }
                PathLine { x: parent.width; y: cellSize * 9 }
                PathMove { x: 0; y: cellSize * 10 }
                PathLine { x: parent.width; y: cellSize * 10 }
                PathMove { x: 0; y: cellSize * 11 }
                PathLine { x: parent.width; y: cellSize * 11 }
                PathMove { x: 0; y: cellSize * 12 }
                PathLine { x: parent.width; y: cellSize * 12 }
                PathMove { x: 0; y: cellSize * 13 }
                PathLine { x: parent.width; y: cellSize * 13 }
                PathMove { x: 0; y: cellSize * 14 }
                PathLine { x: parent.width; y: cellSize * 14 }
            }
        }

        // 绘制天元和星位
        Rectangle {
            x: cellSize * 3 + cellSize / 2 - 4
            y: cellSize * 3 + cellSize / 2 - 4
            width: 8
            height: 8
            radius: 4
            color: "#3a2820"
        }
        Rectangle {
            x: cellSize * 11 + cellSize / 2 - 4
            y: cellSize * 3 + cellSize / 2 - 4
            width: 8
            height: 8
            radius: 4
            color: "#3a2820"
        }
        Rectangle {
            x: cellSize * 3 + cellSize / 2 - 4
            y: cellSize * 11 + cellSize / 2 - 4
            width: 8
            height: 8
            radius: 4
            color: "#3a2820"
        }
        Rectangle {
            x: cellSize * 11 + cellSize / 2 - 4
            y: cellSize * 11 + cellSize / 2 - 4
            width: 8
            height: 8
            radius: 4
            color: "#3a2820"
        }
        Rectangle {
            x: cellSize * 7 + cellSize / 2 - 4
            y: cellSize * 7 + cellSize / 2 - 4
            width: 8
            height: 8
            radius: 4
            color: "#3a2820"
        }
    }

    // 悬停高亮层
    Rectangle {
        id: hoverLayer
        x: hoverCol >= 0 ? hoverCol * cellSize + cellSize / 2 - cellSize / 2.5 : -100
        y: hoverRow >= 0 ? hoverRow * cellSize + cellSize / 2 - cellSize / 2.5 : -100
        width: cellSize / 1.25
        height: cellSize / 1.25
        radius: cellSize / 2.5
        color: "#ffffff"
        opacity: 0.2
        visible: hoverRow >= 0 && hoverCol >= 0

        Behavior on x { SmoothedAnimation { duration: 80 } }
        Behavior on y { SmoothedAnimation { duration: 80 } }
    }

    // 棋子层
    Repeater {
        id: piecesRepeater
        model: piecesModel

        PieceItem {
            x: col * cellSize + cellSize / 2
            y: row * cellSize + cellSize / 2
            pieceSize: cellSize * 0.8
            isBlack: isPlayerPiece
            isLastMove: isLast
        }
    }

    // 获胜连线高亮
    Canvas {
        id: winLineCanvas
        anchors.fill: parent

        onPaint: {
            if (highlightModel.count < 2) return

            var ctx = getContext("2d")
            ctx.lineWidth = 6
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

    // 鼠标处理
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onMouseXChanged: updateHover(mouseX, mouseY)
        onMouseYChanged: updateHover(mouseX, mouseY)

        function updateHover(mx, my) {
            var gridX = mx - cellSize / 2
            var gridY = my - cellSize / 2

            var newCol = Math.floor(gridX / cellSize)
            var newRow = Math.floor(gridY / cellSize)

            if (newCol >= 0 && newCol < gridSize && newRow >= 0 && newRow < gridSize) {
                hoverRow = newRow
                hoverCol = newCol
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

    // 放置棋子
    function placePiece(row, col, isPlayer) {
        piecesModel.append({
            "row": row,
            "col": col,
            "isPlayerPiece": isPlayer,
            "isLast": true
        })

        // 更新之前的最后一步
        for (var i = 0; i < piecesModel.count - 1; i++) {
            piecesModel.setProperty(i, "isLast", false)
        }
    }

    // 清空棋盘
    function clearBoard() {
        piecesModel.clear()
        highlightModel.clear()
        winLineCanvas.requestPaint()
    }

    // 高亮获胜连线
    function highlightWin(winner) {
        // 这里需要从 C++ 获取获胜的棋子位置
        // 暂时留空，由 C++ 填充
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

// ===== 棋子组件 =====
PieceItem {
    id: pieceItem
}

Rectangle {
    id: pieceItem
    width: pieceSize
    height: pieceSize
    radius: pieceSize / 2
    x: 0
    y: 0
    property int pieceSize: 40
    property bool isBlack: true
    property bool isLastMove: false

    // 棋子渐变
    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: isBlack ? "#4a4a4a" : "#f0f0f0"
        }
        GradientStop {
            position: 0.5
            color: isBlack ? "#2a2a2a" : "#ffffff"
        }
        GradientStop {
            position: 1.0
            color: isBlack ? "#1a1a1a" : "#d0d0d0"
        }
    }

    // 最后一步标记
    Rectangle {
        width: pieceSize * 0.3
        height: width
        radius: width / 2
        color: isBlack ? "#ffffff" : "#000000"
        anchors.centerIn: parent
        opacity: 0.7
    }

    // 落下动画
    scale: 0
    Behavior on scale {
        NumberAnimation {
            from: 0
            to: 1
            duration: 200
            easing.type: Easing.OutBack
        }
    }

    Component.onCompleted: {
        scale = 1
    }

    // 阴影
    layer.effect: DropShadow {
        color: "#000000"
        radius: 4
        samples: 8
        spread: 0.3
    }
}
