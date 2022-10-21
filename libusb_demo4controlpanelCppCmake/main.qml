import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    width: 640
    height: 480

    visible: true
    title: qsTr("Hello World")
    Item {
        id: id_rootItem
        objectName: "masterItem"

        Row {
            Button {
                id: id_button01
                objectName: "buttonConnect"
                text: "connect"

                signal sigQmlButtonClick(string msg)
                onClicked: {
                    console.log("connect Button clicked...")
                    id_button01.sigQmlButtonClick("connect")
                }
            }

            Button {
                id: id_buttonClose
                objectName: "buttonClose"
                text: "Close"
                onClicked: {
                    console.log("close clicked...")
                    id_button01.sigQmlButtonClick("close")
                }
            }

            Button {
                id: id_buttonLight
                objectName: "buttonLight"
                text: "lightSet"
                signal sigQmlButtonClick(string msg)
                onClicked: {
                    console.log("Button clicked...")
                    id_buttonLight.sigQmlButtonClick("12")
                }
            }
        }
    }
}
