import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import QtQuick 2.9
import org.graft 1.0
import "../"

GraftApplicationWindow {
    id: root
    title: qsTr("POS")

    Loader {
        id: drawerLoader
        onLoaded: {
            drawerLoader.item.pushScreen = screenTransitions()
            drawerLoader.item.balanceInGraft = GraftClient.balance(GraftClientTools.UnlockedBalance)
        }
    }

    footer: Loader {
        id: footerLoader
        onLoaded: footerLoader.item.pushScreen = screenTransitions()
    }

    Component.onCompleted: {
        if (Qt.platform.os === "ios") {
            footerLoader.source = "qrc:/pos/GraftToolBar.qml"
        } else {
            drawerLoader.source = "qrc:/pos/GraftMenu.qml"
        }
    }

    Connections {
        target: GraftClient

        onErrorReceived: {
            if (message !== "") {
                messageDialog.title = qsTr("Network Error")
                messageDialog.text = message
            } else {
                messageDialog.title = qsTr("Sale failed!")
                messageDialog.text = qsTr("Sale request failed.\nPlease try again.")
            }
            messageDialog.open()
        }
    }

    MessageDialog {
        id: messageDialog
        title: qsTr("Sale failed!")
        icon: StandardIcon.Warning
        text: qsTr("Sale request failed.\nPlease try again.")
        standardButtons: MessageDialog.Ok
        onAccepted: clearChecked()
    }

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: mainScreen
        focus: true
        Keys.onReleased: {
            if (!busy && (event.key === Qt.Key_Back || event.key === Qt.Key_Escape)) {
                if (currentItem.isMenuActive === false) {
                    pop()
                    event.accepted = true
                }
            }
        }

        onCurrentItemChanged: {
            if (drawerLoader.status === Loader.Ready) {
                drawerLoader.item.interactive = currentItem.isMenuActive
            }
        }
    }

    ProductScreen {
        id: mainScreen
        pushScreen: screenTransitions()
    }

    function screenTransitions() {
        var transitionsMap = {}
        transitionsMap["showMenu"] = showMenu
        transitionsMap["hideMenu"] = hideMenu
        transitionsMap["openEditingItemScreen"] = openEditingItemScreen
        transitionsMap["openQuickDealScreen"] = openQuickDealScreen
        transitionsMap["initializingCheckout"] = openCartScreen
        transitionsMap["openWalletScreen"] = openInfoWalletScreen
        transitionsMap["openMainScreen"] = openMainScreen
        transitionsMap["openSettingsScreen"] = openSettingsScreen
        transitionsMap["openPaymentScreen"] = openPaymentScreen
        transitionsMap["openAddAccountScreen"] = openAddAccountScreen
        transitionsMap["goBack"] = turnBack
        transitionsMap["clearChecked"] = clearChecked
        return transitionsMap
    }

    function showMenu() {
        drawerLoader.item.open()
    }

    function hideMenu() {
        drawerLoader.item.close()
    }

    function openEditingItemScreen(index) {
        stack.push("qrc:/pos/EditingItemScreen.qml", {"pushScreen": screenTransitions(),
                   "currencyModel": CurrencyModel, "index": index})
    }

    function openQuickDealScreen() {
        stack.push("qrc:/pos/QuickDialScreen.qml", {"pushScreen": screenTransitions(),
                   "textLabel": qsTr("Quick Dial"), "currencyModel": CurrencyModel})
    }

    function openCartScreen() {
        stack.push("qrc:/pos/CartScreen.qml", {"pushScreen": screenTransitions(),
                   "price": ProductModel.totalCost()})
    }

    function openPaymentScreen() {
        stack.push("qrc:/PaymentScreen.qml", {"pushScreen": clearChecked,
                   "title": qsTr("Cart"), "textLabel": qsTr("Checkout complete!"),
                   "isSpacing": false})
    }

    function openInfoWalletScreen() {
        selectButton("Wallet")
        stack.push("qrc:/pos/InfoWalletScreen.qml", {"pushScreen": screenTransitions(),
                   "amountUnlockGraft": GraftClient.balance(GraftClientTools.UnlockedBalance),
                   "amountLockGraft": GraftClient.balance(GraftClientTools.LockedBalance)})
    }

    function openMainScreen() {
        selectButton("Store")
        stack.pop(mainScreen)
    }

    function openSettingsScreen() {
        selectButton("Settings")
        stack.push("qrc:/pos/SettingsScreen.qml", {"pushScreen": screenTransitions()})
    }

    function openAddAccountScreen() {
        stack.push("qrc:/AddAccountScreen.qml", {"pushScreen": screenTransitions(),
                   "coinModel": CoinModel})
    }

    function turnBack() {
        stack.pop()
    }

    function clearChecked() {
        selectButton("Store")
        ProductModel.clearSelections()
        stack.pop(mainScreen)
    }

    function selectButton(name)
    {
        if (Qt.platform.os === "ios") {
            footerLoader.item.seclectedButtonChanged(name)
        }
    }
}
