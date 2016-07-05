/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chat.h"
#include "remoteselector.h"
#include "chatserver.h"
#include "chatclient.h"

#include <qbluetoothuuid.h>
#include <qbluetoothserver.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothlocaldevice.h>

#include <QTimer>

#include <QDebug>

static const QLatin1String serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264e9c8");

Chat::Chat(QObject *parent)
    : currentAdapterIndex(0)
{
    Q_UNUSED(parent)

    localAdapters = QBluetoothLocalDevice::allDevices();
    qDebug() << "localAdapter: " << localAdapters.count();

    QBluetoothLocalDevice localDevice;
    localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

    QBluetoothAddress adapter = QBluetoothAddress();
    remoteSelector = new RemoteSelector(adapter, this);
    connect(remoteSelector, SIGNAL(newServiceFound()), this, SLOT(newServiceFound()));

    server = new ChatServer(this);
    connect(server, SIGNAL(clientConnected(QString)), this, SIGNAL(connected(QString)));
    connect(server, SIGNAL(clientDisconnected(QString)), this, SIGNAL(disconnected(QString)));
    connect(server, SIGNAL(messageReceived(QString,QString)),
            this, SIGNAL(showMessage(QString,QString)));
    connect(this, SIGNAL(sendMessage(QString)), server, SLOT(sendMessage(QString)));
    connect(this, SIGNAL(disconnect()),
            server, SLOT(disconnect()));
    server->startServer();

    localName = QBluetoothLocalDevice().name();
}

Chat::~Chat()
{
    qDeleteAll(clients);
    delete server;
}

void Chat::connectToDevice(QString name)
{
    qDebug() << "Connecting to " << name;

    // Trying to get the service
    QBluetoothServiceInfo service;
    QMapIterator<QString, QBluetoothServiceInfo> i(remoteSelector->m_discoveredServices);
    bool found = false;
    while (i.hasNext()){
        i.next();
        QString key = i.key();
        if ( key == name ) {
            qDebug() << "The device is found";
            service = i.value();
            qDebug() << "value: " << i.value().device().address();
            found = true;
            break;
        }
    }

    if ( found ) {
        qDebug() << "Going to create client";
        ChatClient *client = new ChatClient(this);
        qDebug() << "Connecting...";

        connect(client, SIGNAL(messageReceived(QString,QString)),
                this, SIGNAL(showMessage(QString,QString)));
        connect(client, SIGNAL(disconnected()), this, SIGNAL(clientDisconnected()));
        connect(client, SIGNAL(disconnected()), this, SLOT(clientIsDisconnected()));
        connect(client, SIGNAL(connected(QString)), this, SIGNAL(connected(QString)));
        connect(this, SIGNAL(sendMessage(QString)), client, SLOT(sendMessage(QString)));
        connect(this, SIGNAL(disconnect()), client, SLOT(disconnect()));

        qDebug() << "Start client";
        client->startClient(service);

        clients.append(client);
    }
}

int Chat::adapterFromUserSelection() const
{
    int result = 0;
    QBluetoothAddress newAdapter = localAdapters.at(0).address();
    return result;
}

void Chat::clientIsDisconnected()
{
    ChatClient *client = qobject_cast<ChatClient *>(sender());
    if (client) {
        clients.removeOne(client);
        client->deleteLater();
    }
}

void Chat::searchForDevices()
{      
    qDebug() << "search for devices!";
    if ( remoteSelector ) {
        delete remoteSelector;
        remoteSelector = NULL;
    }

    QBluetoothAddress adapter = QBluetoothAddress();
    remoteSelector = new RemoteSelector(adapter, this);

    connect(remoteSelector, SIGNAL(newServiceFound()), this, SLOT(newServiceFound()));

    remoteSelector->m_discoveredServices.clear();
    remoteSelector->startDiscovery(QBluetoothUuid(serviceUuid));
    connect(remoteSelector, SIGNAL(finished()), this, SIGNAL(discoveryFinished()));
}

void Chat::stopSearch()
{
    qDebug() << "Going to stop discovery...";
    if ( remoteSelector ) {
        remoteSelector->stopDiscovery();
    }
}

void Chat::newServiceFound()
{
    qDebug() << "newServiceFound";

    // get all of the found devices
    QStringList list;

    QMapIterator<QString, QBluetoothServiceInfo> i(remoteSelector->m_discoveredServices);
    while (i.hasNext()){
        i.next();
        qDebug() << "key: " << i.key();
        qDebug() << "value: " << i.value().device().address();
        list << i.key();
    }

    qDebug() << "list count: "  << list.count();

    emit newServicesFound(list);
}
