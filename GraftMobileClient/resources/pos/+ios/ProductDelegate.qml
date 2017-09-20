import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.2
import com.graft.design 1.0

Rectangle {
    height: layout.height
    color: mouseArea.pressed || selectState ? ColorFactory.color(DesignFactory.Highlighting) : "transparent"

    property real productPrice
    property bool selectState: false
    property alias productImage: picture.source
    property alias productName: productText.text

    ColumnLayout {
        id: layout
        anchors {
            left: parent.left
            right: parent.right
        }

        RowLayout {
            spacing: 16
            Layout.topMargin: 6
            Layout.rightMargin: 12
            Layout.leftMargin: 12

            OpacityMask {
                id: opacityMask
                Layout.preferredWidth: 50
                Layout.preferredHeight: 50
                source: picture
                maskSource: circle

                Rectangle {
                    id: circle
                    width: picture.width
                    height: picture.height
                    radius: picture.width / 2
                    visible: false
                }

                Image {
                    id: picture
                    width: 50
                    height: 55
                    visible: false
                }
            }

            Text {
                id: productText
                Layout.fillWidth: true
                color: ColorFactory.color(DesignFactory.MainText)
                font {
                    family: "Liberation Sans"
                    pointSize: 13
                }
            }

            Text {
                id: price
                text: "$ " + productPrice
                color: ColorFactory.color(DesignFactory.MainText)
                font {
                    family: "Liberation Sans"
                    pointSize: 13
                }
            }
        }

        Rectangle {
            Layout.preferredHeight: 1.6
            Layout.alignment: Qt.AlignBottom
            Layout.fillWidth: true
            color: ColorFactory.color(DesignFactory.AllocateLine)
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            ProductModel.changeSelection(index)
        }
    }
}
