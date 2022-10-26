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
        Column {
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
                        console.log("light set clicked...")
                        id_buttonLight.sigQmlButtonClick("light:12")
                    }
                }
                Button {
                    id: id_buttonLightOff
                    objectName: "buttonLightOff"
                    text: "lightsOff"
                    onClicked: {
                        console.log("light off clicked...")
                        id_buttonLight.sigQmlButtonClick("light:0")
                    }
                }
                Button {
                    id: id_buttonLightGet
                    objectName: "buttonLightGet"
                    text: "lightsGet"
                    onClicked: {
                        console.log("light get clicked...")
                        id_buttonLight.sigQmlButtonClick("light:-1")
                    }
                }
            }
            Row {
                Button {
                    id: id_buttonSliderSet
                    objectName: "buttonSliderSet" + text
                    text: "SliderSet"
                    onClicked: {
                        console.log(text + "light get clicked...")
                        id_buttonLight.sigQmlButtonClick("slider:1,1")
                    }
                }
            }
            Row {
                Button {
                    id: id_buttongetVersion
                    objectName: "button" + text
                    text: "getVersion"
                    onClicked: {
                        console.log(text + " clicked...")
                        id_buttonLight.sigQmlButtonClick("version:1,1")
                    }
                }
            }
            Row {
                Button {
                    id: id_buttongetUuid
                    objectName: "button" + text
                    text: "getUuid"
                    onClicked: {
                        console.log(text + " clicked...")
                        id_buttonLight.sigQmlButtonClick("getuuid:2,1")
                    }
                }
            }
            Row {
                Button {
                    id: id_buttongetDiag
                    objectName: "button" + text
                    text: "getDiag"
                    onClicked: {
                        console.log(text + " clicked...")
                        id_buttonLight.sigQmlButtonClick("getdiag:2,1")
                    }
                }
            }
            Row {
                Button {
                    id: id_buttonUpgrade
                    objectName: "button" + text
                    text: "firmwareUpgrade"
                    onClicked: {
                        console.log(text + " clicked...")
                        id_buttonLight.sigQmlButtonClick("firmwareUpgrade:2,1")
                    }
                }
            }
        }
    }
}