import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// 历史对局弹框 QML 版本
// historyBridge 对象由 C++ 注入，提供对局列表模型
Rectangle {
    id: root
    width: 640
    height: 480
    color: "#ecf1f7"
    radius: 16

    signal closed()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 14

        Text {
            text: qsTr("历史对局")
            font.pixelSize: 26
            font.bold: true
            color: "#293446"
        }

        // 搜索框
        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("搜索关键词...")
            font.pixelSize: 16
        }

        // 对局列表
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: typeof historyBridge !== "undefined" ? historyBridge.filteredModel(searchField.text) : null

            delegate: Rectangle {
                width: listView.width
                height: 60
                color: index % 2 === 0 ? "#f5f8fc" : "#edf1f7"
                radius: 8

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 16

                    Text {
                        text: model.finishedAt
                        font.pixelSize: 14
                        color: "#5e6a7c"
                    }

                    Text {
                        text: model.playerWon ? qsTr("胜") : qsTr("负")
                        font.pixelSize: 16
                        font.bold: true
                        color: model.playerWon ? "#2e7d32" : "#c62828"
                    }

                    Text {
                        text: qsTr("%1 步").arg(model.moveCount)
                        font.pixelSize: 14
                        color: "#293446"
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Button {
            text: qsTr("关闭")
            Layout.alignment: Qt.AlignRight
            onClicked: root.closed()
        }
    }
}
