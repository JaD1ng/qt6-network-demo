import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Effects
import NetworkDemo 1.0

Item {
    TCPServerController {
        id: serverController
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
                    text: "服务器控制"
                    font.pixelSize: 16
                    font.bold: true
                    color: theme.primary
                }

                RowLayout {
                    spacing: 12

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
                        Layout.preferredWidth: 120
                        enabled: !serverController.isListening
                        font.pixelSize: 14
                        selectByMouse: true
                    }

                    Button {
                        text: serverController.isListening ? "⏹ 停止服务器" : "▶ 启动服务器"
                        highlighted: serverController.isListening
                        Material.background: serverController.isListening ? theme.danger : theme.primary
                        font.pixelSize: 14
                        onClicked: {
                            if (serverController.isListening) {
                                serverController.stopServer()
                            } else {
                                serverController.startServer(parseInt(portField.text))
                            }
                        }
                    }

                    // 状态徽章
                    Rectangle {
                        Layout.preferredWidth: Math.max(statusLabel.implicitWidth + 32, 100)
                        Layout.preferredHeight: 36
                        radius: 18
                        color: serverController.isListening ? "#E8F5E9" : "#F5F5F5"
                        border.width: 1
                        border.color: serverController.isListening ? theme.accent : "#E0E0E0"

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 8

                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                color: serverController.isListening ? theme.accent : "#9E9E9E"
                            }

                            Label {
                                id: statusLabel
                                text: "在线: " + serverController.clientCount
                                font.pixelSize: 14
                                font.bold: true
                                color: serverController.isListening ? theme.accent : "#757575"
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
                        onClicked: serverController.clearLog()
                    }
                }
            }
        }

        // 消息发送卡片
        Pane {
            Layout.fillWidth: true
            Material.elevation: 2
            padding: 20
            enabled: serverController.isListening

            background: Rectangle {
                color: theme.surface
                radius: 8
                opacity: serverController.isListening ? 1.0 : 0.5
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
                        placeholderText: "输入要广播的消息..."
                        Layout.fillWidth: true
                        font.pixelSize: 14
                        selectByMouse: true
                        onAccepted: {
                            if (broadcastButton.enabled) {
                                broadcastButton.clicked()
                            }
                        }
                    }

                    Button {
                        id: broadcastButton
                        text: "广播消息"
                        highlighted: true
                        enabled: messageField.text.length > 0
                        Material.background: theme.accent
                        font.pixelSize: 14
                        onClicked: {
                            serverController.broadcastMessage(messageField.text)
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
                        text: serverController.log
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
