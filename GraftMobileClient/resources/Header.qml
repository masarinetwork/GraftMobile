import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

Rectangle {
    height: 60
    color: "#707070"

    signal menuIconClicked()

    property alias headerText: headerText.text
    property alias cartEnable: cartIcon.visible
    property alias countOfSelectedProducts: countSelectedProducts.text
    property bool isMenuState: true

    onIsMenuStateChanged: {
        if (isMenuState) {
            menuIcon.source = "qrc:/imgs/menu_icon.png"
        } else {
            menuIcon.source = "qrc:/imgs/back.png"
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 15
        anchors.rightMargin: 15

        Image {
            id: menuIcon
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            Layout.alignment: Qt.AlignLeft
            source: "qrc:/imgs/menu_icon.png"

            MouseArea {
                anchors.fill: parent
                onClicked: menuIconClicked()
            }
        }

        Text {
            id: headerText
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 15
            color: "white"
        }

        Item {
            Layout.preferredHeight: 20
            Layout.preferredWidth: 20

            Image {
                id: cartIcon
                width: 20
                height: 20
                anchors {
                    top: parent.top
                    right: parent.right
                }
                source: "qrc:/imgs/cart_icon.png"

                Rectangle {
                    id: circle
                    width: picture.width
                    height: picture.height
                    radius: picture.width / 2
                    visible: false
                }

                Rectangle {
                    id: picture
                    anchors {
                        verticalCenter: parent.top
                        verticalCenterOffset: 2
                        horizontalCenter: parent.right
                        horizontalCenterOffset: -2
                    }
                    width: 12
                    height: 12
                    color: "white"
                    visible: false

                    Text {
                        id: countSelectedProducts
                        anchors.centerIn: parent
                        color: "#707070"
                        font.pointSize: 8
                    }
                }

                OpacityMask {
                    anchors.fill: picture
                    source: picture
                    maskSource: circle
                }
            }
        }
    }
}
