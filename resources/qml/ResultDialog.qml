import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// 结算弹框 QML 版本
Rectangle {
    id: root
    width: 420
    height: 300
    color: "#ecf1f7"
    radius: 20

    // 属性：由 C++ 设置
    property bool playerWon: true
    property int moveCount: 0
    property int playerWins: 0
    property int aiWins: 0

    signal accepted()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 18

        Text {
            text: root.playerWon ? qsTr("\uD83C\uDFC6 你赢了！") : qsTr("AI 获胜")
            font.pixelSize: 32
            font.bold: true
            color: root.playerWon ? "#2e7d32" : "#c62828"
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        Text {
            text: qsTr("本局步数：%1").arg(root.moveCount)
            font.pixelSize: 18
            color: "#293446"
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        Text {
            text: qsTr("累计战绩  玩家 %1 : %2 AI").arg(root.playerWins).arg(root.aiWins)
            font.pixelSize: 16
            color: "#5e6a7c"
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        Item { Layout.fillHeight: true }

        Button {
            text: qsTr("确定")
            Layout.alignment: Qt.AlignHCenter
            highlighted: true
            onClicked: root.accepted()
        }
    }
}
