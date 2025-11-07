import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 1000
    height: 750
    title: "TCP/UDP 通信 demo"

    // Material Design 主题配置
    Material.theme: Material.Light
    Material.accent: Material.Blue
    Material.primary: Material.Blue

    // 主题颜色定义
    QtObject {
        id: theme
        readonly property color primary: "#2196F3"
        readonly property color primaryDark: "#1976D2"
        readonly property color accent: "#4CAF50"
        readonly property color danger: "#F44336"
        readonly property color warning: "#FF9800"
        readonly property color background: "#F5F5F5"
        readonly property color surface: "#FFFFFF"
        readonly property color textPrimary: "#212121"
        readonly property color textSecondary: "#757575"
        readonly property color divider: "#BDBDBD"
    }

    background: Rectangle {
        color: theme.background
    }

    header: TabBar {
        id: tabBar
        width: parent.width
        Material.elevation: 4

        background: Rectangle {
            color: theme.surface

            Rectangle {
                width: parent.width
                height: 1
                anchors.bottom: parent.bottom
                color: theme.divider
            }
        }

        TabButton {
            text: "TCP Server"
            font.pixelSize: 14
            font.bold: tabBar.currentIndex === 0
        }
        TabButton {
            text: "TCP Client"
            font.pixelSize: 14
            font.bold: tabBar.currentIndex === 1
        }
        TabButton {
            text: "UDP"
            font.pixelSize: 14
            font.bold: tabBar.currentIndex === 2
        }
    }

    StackLayout {
        anchors.fill: parent
        anchors.margins: 16
        currentIndex: tabBar.currentIndex

        TCPServerPage {
            id: tcpServerPage
        }

        TCPClientPage {
            id: tcpClientPage
        }

        UDPPage {
            id: udpPage
        }
    }
}
