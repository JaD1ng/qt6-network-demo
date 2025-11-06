import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 900
    height: 700
    title: "TCP/UDP 通信 demo"

    header: TabBar {
        id: tabBar
        width: parent.width

        TabButton {
            text: "TCP Server"
        }
        TabButton {
            text: "TCP Client"
        }
        TabButton {
            text: "UDP"
        }
    }

    StackLayout {
        anchors.fill: parent
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
