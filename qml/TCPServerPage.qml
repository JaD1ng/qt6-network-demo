import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NetworkDemo 1.0

Item {
    TCPServerController {
        id: serverController
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // 控制区域
        GroupBox {
            title: "服务器控制"
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: 10

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
                    enabled: !serverController.isListening
                }

                Button {
                    text: serverController.isListening ? "停止服务器" : "启动服务器"
                    onClicked: {
                        if (serverController.isListening) {
                            serverController.stopServer()
                        } else {
                            serverController.startServer(parseInt(portField.text))
                        }
                    }
                }

                Label {
                    text: "在线客户端: " + serverController.clientCount
                    Layout.leftMargin: 20
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "清空日志"
                    onClicked: serverController.clearLog()
                }
            }
        }

        // 消息发送区域
        GroupBox {
            title: "发送消息"
            Layout.fillWidth: true
            enabled: serverController.isListening

            RowLayout {
                anchors.fill: parent
                spacing: 10

                TextField {
                    id: messageField
                    placeholderText: "输入要发送的消息..."
                    Layout.fillWidth: true
                    onAccepted: broadcastButton.clicked()
                }

                Button {
                    id: broadcastButton
                    text: "广播消息"
                    enabled: messageField.text.length > 0
                    onClicked: {
                        serverController.broadcastMessage(messageField.text)
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
                    text: serverController.log
                    wrapMode: TextEdit.Wrap
                    font.family: "Consolas"
                    font.pixelSize: 12
                    selectByMouse: true
                }
            }
        }
    }
}