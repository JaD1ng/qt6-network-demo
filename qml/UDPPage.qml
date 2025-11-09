import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Effects
import NetworkDemo 1.0

Item {
    UDPController {
        id: udpController
    }

    // 主题颜色
    QtObject {
        id: theme
        readonly property color primary: "#2196F3"
        readonly property color accent: "#4CAF50"
        readonly property color danger: "#F44336"
        readonly property color warning: "#FF9800"
        readonly property color surface: "#FFFFFF"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        // 控制卡片
        Pane {
            Layout.fillWidth: true
            Material.elevation: 2
            padding: 20

            background: Rectangle {
                color: theme.surface
                radius: 8
            }

            ColumnLayout {
                width: parent.width
                spacing: 16

                // 标题
                Label {
                    text: "UDP 控制"
                    font.pixelSize: 16
                    font.bold: true
                    color: theme.primary
                }

                RowLayout {
                    spacing: 12

                    Label {
                        text: "本地端口:"
                        font.pixelSize: 14
                    }

                    TextField {
                        id: localPortField
                        text: "9999"
                        placeholderText: "本地端口"
                        validator: IntValidator {
                            bottom: 1
                            top: 65535
                        }
                        Layout.preferredWidth: 100
                        enabled: !udpController.isBound
                        font.pixelSize: 14
                        selectByMouse: true
                    }

                    Button {
                        text: udpController.isBound ? "解绑" : "绑定"
                        highlighted: udpController.isBound
                        Material.background: udpController.isBound ? theme.danger : theme.primary
                        font.pixelSize: 14
                        onClicked: {
                            if (udpController.isBound) {
                                udpController.unbind()
                            } else {
                                udpController.bind(parseInt(localPortField.text))
                            }
                        }
                    }

                    // 状态徽章
                    Rectangle {
                        Layout.preferredWidth: Math.max(statusLabel.implicitWidth + 32, 120)
                        Layout.preferredHeight: 36
                        radius: 18
                        color: udpController.isBound ? "#E8F5E9" : "#FFF3E0"
                        border.width: 1
                        border.color: udpController.isBound ? theme.accent : theme.warning

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 8

                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                color: udpController.isBound ? theme.accent : theme.warning
                            }

                            Label {
                                id: statusLabel
                                text: udpController.isBound ? "已绑定: " + udpController.localPort : "未绑定"
                                font.pixelSize: 14
                                font.bold: true
                                color: udpController.isBound ? theme.accent : theme.warning
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "清空日志"
                        flat: true
                        Material.foreground: "#757575"
                        font.pixelSize: 13
                        onClicked: udpController.clearLog()
                    }
                }
            }
        }

        // 消息发送卡片
        Pane {
            Layout.fillWidth: true
            Material.elevation: 2
            padding: 20

            background: Rectangle {
                color: theme.surface
                radius: 8
            }

            ColumnLayout {
                width: parent.width
                spacing: 12

                Label {
                    text: "发送消息"
                    font.pixelSize: 16
                    font.bold: true
                    color: theme.primary
                }

                // 目标设置
                RowLayout {
                    spacing: 12

                    Label {
                        text: "目标地址:"
                        font.pixelSize: 14
                    }

                    TextField {
                        id: targetHostField
                        text: "127.0.0.1"
                        placeholderText: "目标IP地址"
                        Layout.preferredWidth: 150
                        font.pixelSize: 14
                        selectByMouse: true
                    }

                    Label {
                        text: "目标端口:"
                        font.pixelSize: 14
                    }

                    TextField {
                        id: targetPortField
                        text: "9999"
                        placeholderText: "目标端口"
                        validator: IntValidator {
                            bottom: 1
                            top: 65535
                        }
                        Layout.preferredWidth: 100
                        font.pixelSize: 14
                        selectByMouse: true
                    }
                }

                // 消息输入和发送按钮
                RowLayout {
                    spacing: 12

                    TextField {
                        id: messageField
                        placeholderText: "输入要发送的消息..."
                        Layout.fillWidth: true
                        font.pixelSize: 14
                        selectByMouse: true
                        onAccepted: {
                            if (sendUnicastButton.enabled) {
                                sendUnicastButton.clicked()
                            }
                        }
                    }

                    Button {
                        id: sendUnicastButton
                        text: "发送单播"
                        highlighted: true
                        enabled: messageField.text.length > 0
                        Material.background: theme.accent
                        font.pixelSize: 14
                        onClicked: {
                            udpController.sendMessage(messageField.text, targetHostField.text, parseInt(targetPortField.text))
                            messageField.clear()
                        }
                    }

                    Button {
                        id: sendBroadcastButton
                        text: "发送广播"
                        highlighted: true
                        enabled: messageField.text.length > 0
                        Material.background: theme.warning
                        font.pixelSize: 14
                        onClicked: {
                            udpController.sendBroadcast(messageField.text, parseInt(targetPortField.text))
                            messageField.clear()
                        }
                    }
                }
            }
        }

        // 日志卡片
        Pane {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Material.elevation: 2
            padding: 0

            background: Rectangle {
                color: theme.surface
                radius: 8
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // 日志标题栏
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    color: "#FAFAFA"
                    radius: 8

                    Rectangle {
                        width: parent.width
                        height: parent.radius
                        anchors.bottom: parent.bottom
                        color: parent.color
                    }

                    Label {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        anchors.verticalCenter: parent.verticalCenter
                        text: "通信日志"
                        font.pixelSize: 16
                        font.bold: true
                        color: theme.primary
                    }
                }

                // 日志内容
                ScrollView {
                    id: logScrollView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    TextArea {
                        id: logTextArea
                        readOnly: true
                        text: udpController.log
                        wrapMode: TextEdit.Wrap
                        font.family: "Consolas, Monaco, monospace"
                        font.pixelSize: 13
                        selectByMouse: true
                        color: "#212121"
                        padding: 16
                        background: Rectangle {
                            color: "#FAFAFA"
                        }

                        onTextChanged: {
                            // 延迟执行滚动操作，确保文本已渲染
                            Qt.callLater(function() {
                                if (logScrollView.ScrollBar.vertical) {
                                    logScrollView.ScrollBar.vertical.position = 1.0 - logScrollView.ScrollBar.vertical.size
                                }
                            })
                        }
                    }
                }
            }
        }
    }
}
