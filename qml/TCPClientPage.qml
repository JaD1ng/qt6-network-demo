import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NetworkDemo 1.0

Item {
    TCPClientController {
        id: clientController
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // 控制区域
        GroupBox {
            title: "客户端控制"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                RowLayout {
                    spacing: 10

                    Label {
                        text: "服务器:"
                    }

                    TextField {
                        id: hostField
                        text: "127.0.0.1"
                        placeholderText: "服务器地址"
                        Layout.preferredWidth: 150
                        enabled: !clientController.isConnected
                    }

                    Label {
                        text: "端口:"
                    }

                    TextField {
                        id: portField
                        text: "8888"
                        placeholderText: "端口号"
                        validator: IntValidator {
                            bottom: 1; top: 65535
                        }
                        Layout.preferredWidth: 100
                        enabled: !clientController.isConnected
                    }

                    Button {
                        text: clientController.isConnected ? "断开连接" : "连接服务器"
                        onClicked: {
                            if (clientController.isConnected) {
                                clientController.disconnectFromServer()
                            } else {
                                clientController.connectToServer(hostField.text, parseInt(portField.text))
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "清空日志"
                        onClicked: clientController.clearLog()
                    }
                }

                RowLayout {
                    spacing: 10

                    CheckBox {
                        id: autoReconnectCheckBox
                        text: "自动重连"
                        checked: clientController.autoReconnect
                        onCheckedChanged: {
                            clientController.autoReconnect = checked
                        }
                    }

                    Label {
                        text: clientController.isConnected ? "状态: 已连接" : "状态: 未连接"
                        color: clientController.isConnected ? "green" : "gray"
                        font.bold: true
                    }
                }
            }
        }

        // 消息发送区域
        GroupBox {
            title: "发送消息"
            Layout.fillWidth: true
            enabled: clientController.isConnected

            RowLayout {
                anchors.fill: parent
                spacing: 10

                TextField {
                    id: messageField
                    placeholderText: "输入要发送的消息..."
                    Layout.fillWidth: true
                    onAccepted: sendButton.clicked()
                }

                Button {
                    id: sendButton
                    text: "发送"
                    enabled: messageField.text.length > 0
                    onClicked: {
                        clientController.sendMessage(messageField.text)
                        messageField.clear()
                    }
                }
            }
        }

        // 日志显示区域
        GroupBox {
            title: "通信日志"
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollView {
                anchors.fill: parent
                clip: true

                TextArea {
                    readOnly: true
                    text: clientController.log
                    wrapMode: TextEdit.Wrap
                    font.family: "Consolas"
                    font.pixelSize: 12
                    selectByMouse: true
                }
            }
        }
    }
}
