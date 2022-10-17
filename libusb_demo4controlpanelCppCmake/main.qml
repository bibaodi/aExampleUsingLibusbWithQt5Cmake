import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Button {
        id: id_button01
        objectName: "buttonObjName"
        text: "Button"
        signal sigQmlButtonClick(string msg)
        onClicked: {
            console.log("Button clicked...")
            id_button01.sigQmlButtonClick("update")
        }
    }
}
