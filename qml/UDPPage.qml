import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NetworkDemo 1.0

Item {
    UDPController {
        id: udpController
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // 控制区域
        GroupBox {
            title: "UDP 控制"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                RowLayout {
                    spacing: 10

                    Label {
                        text: "本地端口:"
                    }

                    TextField {
                        id: localPortField
                        text: "9999"
                        placeholderText: "本地端口"
                        validator: IntValidator {
                            bottom: 1; top: 65535
                        }
                        Layout.preferredWidth: 100
                        enabled: !udpController.isBound
                    }

                    Button {
                        text: udpController.isBound ? "解绑" : "绑定"
                        onClicked: {
                            if (udpController.isBound) {
                                udpController.unbind()
                            } else {
                                udpController.bind(parseInt(localPortField.text))
                            }
                        }
                    }

                    Label {
                        text: udpController.isBound ? "状态: 已绑定 (端口 " + udpController.localPort + ")" : "状态: 未绑定"
                        color: udpController.isBound ? "green" : "gray"
                        font.bold: true
                        Layout.leftMargin: 20
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "清空日志"
                        onClicked: udpController.clearLog()
                    }
                }
            }
        }

        // 消息发送区域
        GroupBox {
            title: "发送消息"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                RowLayout {
                    spacing: 10

                    Label {
                        text: "目标地址:"
                    }

                    TextField {
                        id: targetHostField
                        text: "127.0.0.1"
                        placeholderText: "目标IP地址"
                        Layout.preferredWidth: 150
                    }

                    Label {
                        text: "目标端口:"
                    }

                    TextField {
                        id: targetPortField
                        text: "9999"
                        placeholderText: "目标端口"
                        validator: IntValidator {
                            bottom: 1; top: 65535
                        }
                        Layout.preferredWidth: 100
                    }
                }

                RowLayout {
                    spacing: 10

                    TextField {
                        id: messageField
                        placeholderText: "输入要发送的消息..."
                        Layout.fillWidth: true
                        onAccepted: sendUnicastButton.clicked()
                    }

                    Button {
                        id: sendUnicastButton
                        text: "发送单播"
                        enabled: messageField.text.length > 0
                        onClicked: {
                            udpController.sendMessage(messageField.text, targetHostField.text, parseInt(targetPortField.text))
                            messageField.clear()
                        }
                    }

                    Button {
                        id: sendBroadcastButton
                        text: "发送广播"
                        enabled: messageField.text.length > 0
                        onClicked: {
                            udpController.sendBroadcast(messageField.text, parseInt(targetPortField.text))
                            messageField.clear()
                        }
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
                    text: udpController.log
                    wrapMode: TextEdit.Wrap
                    font.family: "Consolas"
                    font.pixelSize: 12
                    selectByMouse: true
                }
            }
        }
    }
}
