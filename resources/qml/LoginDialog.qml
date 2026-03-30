import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// 登录弹框 QML 版本
// 通过 loginBridge 对象与 C++ 交互
Rectangle {
    id: root
    width: 480
    height: 380
    color: "#ecf1f7"
    radius: 20

    // 信号：用户确认登录
    signal accepted(string userName)
    // 信号：用户取消
    signal rejected()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 16

        Text {
            text: qsTr("登录")
            font.pixelSize: 28
            font.bold: true
            color: "#293446"
        }

        Text {
            text: qsTr("输入用户名后登录")
            font.pixelSize: 14
            color: "#5e6a7c"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        ComboBox {
            id: userNameCombo
            Layout.fillWidth: true
            editable: true
            model: typeof loginBridge !== "undefined" ? loginBridge.recentUsers : []
            implicitHeight: 48
            font.pixelSize: 18
        }

        Text {
            id: hintLabel
            text: userNameCombo.editText.length === 0
                ? qsTr("输入用户名后即可查看历史数据")
                : qsTr("登录后将继续累计对局记录")
            font.pixelSize: 13
            color: "#637080"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
                text: qsTr("退出")
                onClicked: root.rejected()
            }

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("进入对局")
                enabled: userNameCombo.editText.trim().length > 0
                highlighted: true
                onClicked: root.accepted(userNameCombo.editText.trim())
            }
        }
    }
}
