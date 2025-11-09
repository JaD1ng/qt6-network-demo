import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Effects
import NetworkDemo 1.0

Item {
    TCPClientController {
        id: clientController
    }

    // 主题颜色
    QtObject {
        id: theme
        readonly property color primary: "#2196F3"
        readonly property color accent: "#4CAF50"
        readonly property color danger: "#F44336"
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
                    text: "客户端控制"
                    font.pixelSize: 16
                    font.bold: true
                    color: theme.primary
                }

                // 连接设置
                RowLayout {
                    spacing: 12

                    Label {
                        text: "服务器:"
                        font.pixelSize: 14
                    }

                    TextField {
                        id: hostField
                        text: "127.0.0.1"
                        placeholderText: "服务器地址"
                        Layout.preferredWidth: 150
                        enabled: !clientController.isConnected
                        font.pixelSize: 14
                        selectByMouse: true
                    }

                    Label {
                        text: "端口:"
                        font.pixelSize: 14
                    }

                    TextField {
                        id: portField
                        text: "8888"
                        placeholderText: "端口号"
                        validator: IntValidator {
                            bottom: 1
                            top: 65535
                        }
                        Layout.preferredWidth: 100
                        enabled: !clientController.isConnected
                        font.pixelSize: 14
                        selectByMouse: true
                    }

                    Button {
                        text: clientController.isConnected ? "断开连接" : "连接服务器"
                        highlighted: clientController.isConnected
                        Material.background: clientController.isConnected ? theme.danger : theme.primary
                        font.pixelSize: 14
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
                        flat: true
                        Material.foreground: "#757575"
                        font.pixelSize: 13
                        onClicked: clientController.clearLog()
                    }
                }

                // 状态和选项
                RowLayout {
                    spacing: 16

                    CheckBox {
                        id: autoReconnectCheckBox
                        text: "自动重连"
                        checked: clientController.autoReconnect
                        font.pixelSize: 14
                        onCheckedChanged: {
                            clientController.autoReconnect = checked
                        }
                    }

                    // 状态徽章
                    Rectangle {
                        Layout.preferredWidth: Math.max(statusLabel.implicitWidth + 32, 110)
                        Layout.preferredHeight: 36
                        radius: 18
                        color: clientController.isConnected ? "#E8F5E9" : "#FFEBEE"
                        border.width: 1
                        border.color: clientController.isConnected ? theme.accent : theme.danger

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 8

                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                color: clientController.isConnected ? theme.accent : theme.danger

                                SequentialAnimation on opacity {
                                    running: clientController.isConnected
                                    loops: Animation.Infinite
                                    NumberAnimation {
                                        from: 1.0;
                                        to: 0.3; duration: 800
                                    }
                                    NumberAnimation {
                                        from: 0.3;
                                        to: 1.0; duration: 800
                                    }
                                }
                            }

                            Label {
                                id: statusLabel
                                text: clientController.isConnected ? "已连接" : "未连接"
                                font.pixelSize: 14
                                font.bold: true
                                color: clientController.isConnected ? theme.accent : theme.danger
                            }
                        }
                    }
                }
            }
        }

        // 消息发送卡片
        Pane {
            Layout.fillWidth: true
            Material.elevation: 2
            padding: 20
            enabled: clientController.isConnected

            background: Rectangle {
                color: theme.surface
                radius: 8
                opacity: clientController.isConnected ? 1.0 : 0.5
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

                RowLayout {
                    spacing: 12

                    TextField {
                        id: messageField
                        placeholderText: "输入要发送的消息..."
                        Layout.fillWidth: true
                        font.pixelSize: 14
                        selectByMouse: true
                        onAccepted: {
                            if (sendButton.enabled) {
                                sendButton.clicked()
                            }
                        }
                    }

                    Button {
                        id: sendButton
                        text: "发送"
                        highlighted: true
                        enabled: messageField.text.length > 0
                        Material.background: theme.accent
                        font.pixelSize: 14
                        onClicked: {
                            clientController.sendMessage(messageField.text)
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
                        text: clientController.log
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
