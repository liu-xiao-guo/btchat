import QtQuick 2.4
import Ubuntu.Components 1.3
import QtBluetooth 5.3

MainView {
    // objectName for functional testing purposes (autopilot-qt5)
    objectName: "mainView"

    // Note! applicationName needs to match the "name" field of the click manifest
    applicationName: "btchat.xiaoguo"

    width: units.gu(60)
    height: units.gu(85)

    function getBluetoothState(state) {
        switch (state ) {
        case BluetoothSocket.Unconnected:
            return "Unconnected";
        case BluetoothSocket.ServiceLookup:
            return "ServiceLookup";
        case BluetoothSocket.Connecting:
            return "Connecting";
        case BluetoothSocket.Connected:
            return "Connected";
        case BluetoothSocket.Bound:
            return "Bound";
        case BluetoothSocket.Closing:
            return "Closing";
        case BluetoothSocket.Listening:
            return "Listening";
        case BluetoothSocket.ServiceLookup:
            return "ServiceLookup";
        case BluetoothSocket.NoServiceSet:
            return "NoServiceSet";
        default:
            return "Unknow state"
        }
    }

    function appendMessage(msg, alignright) {
        mymodel.append({ "msg": msg, "alignright": alignright} )
        listview.positionViewAtIndex(mymodel.count - 1, ListView.Beginning)
    }

    Connections {
        target: chat
        onConnected: {
            console.log("Connected: " + name)
            appendMessage( "connected to " + name, false )
        }

        onDisconnected: {
            console.log("Disconnected: " + name )
            appendMessage( "disconnected " + name, false )
        }

        onClientDisconnected: {
            console.log("Client Disconnected")
            appendMessage("Client Disconnected", false)
        }

        onShowMessage: {
            console.log("sender: " + sender )
            console.log("message: " + message )
            message = message.replace(/(\r\n|\n|\r)/gm,"");
            var msg = '<font color = "green">' + message + '</font>';
            console.log("msg: " + msg)
            appendMessage(msg, false)
        }

        onNewServicesFound: {
            console.log("new services found!")
            devlist.model = list;
        }

        onDiscoveryFinished: {
            console.log("discovery finished");
            indicator.running = false;
        }
    }

    ListModel {
        id: mymodel
    }

    ListModel {
        id: services
    }

    BluetoothDiscoveryModel {
        id: btModel
        running: false
        discoveryMode: BluetoothDiscoveryModel.FullServiceDiscovery
        onRunningChanged : {
        }

        onErrorChanged: {
        }

        onServiceDiscovered: {
            console.log("service has been found!")
            services.append( {"service": service })
        }
        uuidFilter: "e8e10f95-1a70-4b27-9ccf-02010264e9c8"
    }

    BluetoothSocket {
        id: socket
        connected: true

        onSocketStateChanged: {
            console.log("socketState: " + socketState);
            console.log("Connected to server! ")
            appendMessage( "State: " + getBluetoothState(socketState) )
        }

        onStringDataChanged: {
            console.log("Received data: " )
            var data = "Going to send: " + socket.stringData;
            data = data.substring(0, data.indexOf('\n'))
            console.log(data);
            appendMessage("Received: " + data)
        }
    }

    Component {
        id: highlight
        Rectangle {
            width: devlist.width
            height: devlist.delegate.height
            color: "lightsteelblue"; radius: 5
            Behavior on y {
                SpringAnimation {
                    spring: 3
                    damping: 0.2
                }
            }
        }
    }

    Page {
        id: page
        header: standardHeader

        PageHeader {
            id: standardHeader
            visible: page.header === standardHeader
            title: "Bluetooth chat"
            trailingActionBar.actions: [
                Action {
                    iconName: "edit"
                    text: "Edit"
                    onTriggered: page.header = editHeader
                }
            ]
        }

        PageHeader {
            id: editHeader
            visible: page.header === editHeader
            leadingActionBar.actions: [
                Action {
                    iconName: "back"
                    text: "Back"
                    onTriggered: {
                        page.header = standardHeader
                    }
                }
            ]
            contents: Row {
                id: layout
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                spacing: units.gu(2)

                TextField {
                    id: input
                    width: parent.width*2/3
                    placeholderText: "input words .."
                    text: "I love you!"

                    onAccepted: {
                        console.log("going to send: " + text)
                    }
                }

                Button {
                    text: "Send"
                    width: parent.width - input.width - layout.spacing;
                    onClicked: {
                        console.log("send is clicked")
                        console.log("chat length: " + input.text.length)
                        chat.sendMessage(input.text);
                        appendMessage(input.text, true)
                    }
                }
            }
        }

        Item {
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                top: page.header.bottom
            }

            ActivityIndicator {
                id: indicator
                anchors.centerIn: parent
            }

            Column {
                anchors.fill: parent

                Label {
                    text: "Devices are:"
                    fontSize: "x-large"
                }

                ListView {
                    id: devlist
                    width: parent.width
                    height: parent.height/4
                    model: services
                    highlight: highlight
                    delegate: Label {
                        width: parent.width
                        text: modelData
                        fontSize: "x-large"
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                console.log("it is selected")
                                chat.connectToDevice(modelData)
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: units.gu(0.4)
                    color: "green"
                }

                ListView {
                    id: listview
                    width: parent.width
                    height: parent.height - devlist.height
                    model: mymodel
                    delegate: Item {
                        width: page.width
                        height: txt.height * 1.2 /*+ div.height*/

                        Rectangle {
                            width: txt.contentWidth
                            height: parent.height
                            color: alignright? "green" : "white"
                            radius: units.gu(0.5)
                            anchors.right: alignright ? parent.right : undefined
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Label {
                            id: txt
                            width: parent.width*0.7
                            text: msg
                            anchors.right: alignright ? parent.right : undefined
                            horizontalAlignment: alignright ? Text.AlignRight :
                                                              Text.AlignLeft
                            fontSize: "large"
                            anchors.verticalCenter: parent.verticalCenter
                            wrapMode: Text.WordWrap
                        }

//                        Rectangle {
//                            id: div
//                            width: parent.width
//                            height: units.gu(0.2)
//                            color: "blue"
//                        }
                    }
                }
            }


            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: units.gu(1)
                spacing: units.gu(2)

                Button {
                    text: "Search"
                    onClicked: {
                        console.log("btModel.running: " + btModel.running )
                        var list = []
                        devlist.model = list;
                        chat.searchForDevices()
                        indicator.running = true
                    }
                }

                Button {
                    text: "Stop search"
                    onClicked: {
                        console.log("Stop is clicked!")
                        chat.stopSearch();
                        indicator.running = false
                    }
                }

                Button {
                    text: "Disconnect"
                    onClicked: {
                        console.log("Disconnect is clicked!")
                        chat.disconnect();
                    }
                }
            }
        }
    }
}

