import QtQuick 2.15

/**
 * WinRateChart - 胜率走势图组件
 * 使用 Canvas 绘制双线胜率图
 */
Rectangle {
    id: root

    // 背景
    radius: 12
    color: "#1a1a2e"

    // 画布
    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: 10

        onPaint: {
            drawChart(context2D, width, height)
        }
    }

    // 绘制图表
    function drawChart(ctx, w, h) {
        // 背景
        ctx.fillStyle = "#1a1a2e"
        ctx.fillRect(0, 0, w, h)

        // 网格线
        ctx.strokeStyle = "#2a2a4a"
        ctx.lineWidth = 1
        for (var i = 0; i <= 10; i++) {
            var y = i * h / 10
            ctx.beginPath()
            ctx.moveTo(0, y)
            ctx.lineTo(w, y)
            ctx.stroke()
        }

        // 获取数据
        var playerHistory = gameBridge.getPlayerRateHistory()
        var aiHistory = gameBridge.getAiRateHistory()

        // 绘制玩家胜率线（青色）
        if (playerHistory.length > 1) {
            ctx.strokeStyle = "#4ecdc4"
            ctx.lineWidth = 2
            ctx.beginPath()
            for (var i = 0; i < playerHistory.length; i++) {
                var x = i * w / Math.max(playerHistory.length - 1, 1)
                var y = h - (playerHistory[i] * h / 100)
                if (i === 0) ctx.moveTo(x, y)
                else ctx.lineTo(x, y)
            }
            ctx.stroke()
        }

        // 绘制 AI 胜率线（红色）
        if (aiHistory.length > 1) {
            ctx.strokeStyle = "#ff6b6b"
            ctx.lineWidth = 2
            ctx.beginPath()
            for (var i = 0; i < aiHistory.length; i++) {
                var x = i * w / Math.max(aiHistory.length - 1, 1)
                var y = h - (aiHistory[i] * h / 100)
                if (i === 0) ctx.moveTo(x, y)
                else ctx.lineTo(x, y)
            }
            ctx.stroke()
        }

        // 50% 参考线
        ctx.strokeStyle = "#555566"
        ctx.lineWidth = 1
        ctx.beginPath()
        ctx.moveTo(0, h / 2)
        ctx.lineTo(w, h / 2)
        ctx.stroke()
    }

    // 请求重绘
    function requestPaint() {
        canvas.requestPaint()
    }
}
