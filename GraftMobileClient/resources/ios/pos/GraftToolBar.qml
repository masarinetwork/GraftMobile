import QtQuick 2.9
import QtQuick.Layouts 1.3
import com.graft.design 1.0
import "../components"

Rectangle {
    property var pushScreen

    height: 49
    color: ColorFactory.color(DesignFactory.IosNavigationBar)

    RowLayout {
        spacing: 35
        anchors.centerIn: parent

        ToolBarButton  {
            text: qsTr("Store")
            source: "qrc:/imgs/storeIos.png"
            onClicked: {
                pushScreen.openProductScreen()
            }
        }

        ToolBarButton {
            text: qsTr("Wallet")
            source: "qrc:/imgs/walletIos.png"
            onClicked: {
                pushScreen.openWalletScreen()
            }
        }

        ToolBarButton {
            text: qsTr("Settings")
            source: "qrc:/imgs/configIos.png"
            onClicked: {
                pushScreen.openSettingsScreen()
            }
        }

        ToolBarButton {
            text: qsTr("About")
            source: "qrc:/imgs/infoIos.png"
            onClicked: {
                pushScreen.openAboutScreen()
            }
        }
    }
}