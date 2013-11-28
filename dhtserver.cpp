#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>

#include "dhtserver.h"
#include "util.h"
#include "ui_dhtserver.h"

HostNameLookup::HostNameLookup(quint16 p) {
    this->port = p;
}

DHTServer::DHTServer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DHTServer)
{

    QGroupBox *keyValEnterGroup = new QGroupBox(tr("Insert a key value pair"));
    QLabel *keyLabel = new QLabel("Key:", this);
    keyInsertInput = new QLineEdit(this);
    QLabel *valLabel = new QLabel("Value:", this);
    valInsertInput = new QLineEdit(this);
    insertBtn = new QPushButton("Insert", this);
    connect(insertBtn, SIGNAL(clicked()), this, SLOT(keyValInsertionHandler()));
    QGridLayout *keyValLayout = new QGridLayout;
    keyValLayout->addWidget(keyLabel, 0, 0);
    keyValLayout->addWidget(keyInsertInput, 0, 1);
    keyValLayout->addWidget(valLabel, 1, 0);
    keyValLayout->addWidget(valInsertInput, 1, 1);
    keyValLayout->addWidget(insertBtn, 2, 0, 1, 2);
    keyValEnterGroup->setLayout(keyValLayout);


    QGroupBox *nodeEnterGroup = new QGroupBox(tr("Enter a node"));
    QLabel *nodeEnterLabel = new QLabel("Node:", this);
    nodeEnterInput = new QLineEdit(this);
    nodeJoinBtn = new QPushButton("Join", this);
    connect(nodeJoinBtn, SIGNAL(clicked()), this, SLOT(nodeJoinBtnClickedHandler()));
    QGridLayout *nodeEnterLayout = new QGridLayout;
    nodeEnterLayout->addWidget(nodeEnterLabel, 0, 0);
    nodeEnterLayout->addWidget(nodeEnterInput, 0, 1);
    nodeEnterLayout->addWidget(nodeJoinBtn, 1, 0, 1, 2);
    nodeEnterGroup->setLayout(nodeEnterLayout);


    QGroupBox *keySearchGroup = new QGroupBox(tr("Find a key"));
    QLabel *keySearchLabel = new QLabel("Key:", this);
    keySearchInput = new QLineEdit(this);
    QLabel *valFoundLabel = new QLabel("Val:", this);
    valDisplay = new QLineEdit(this);
    valDisplay->setReadOnly(true);
    QPalette p = valDisplay->palette();
    p.setColor(QPalette::Base, QColor(240, 240, 255));
    valDisplay->setPalette(p);
    keySearchBtn = new QPushButton("Search");
    QGridLayout *keySearchLayout = new QGridLayout;
    keySearchLayout->addWidget(keySearchLabel, 0, 0);
    keySearchLayout->addWidget(keySearchInput, 0, 1);
    keySearchLayout->addWidget(valFoundLabel, 1, 0);
    keySearchLayout->addWidget(valDisplay, 1, 1);
    keySearchLayout->addWidget(keySearchBtn, 2, 0, 1, 2);
    keySearchGroup->setLayout(keySearchLayout);

    QGroupBox *successorGroup = new QGroupBox(tr("Successors"));
    successorDisplay = new QTextEdit(this);
    successorDisplay->setReadOnly(true);
    QGridLayout *successorDisplayLayout = new QGridLayout;
    successorDisplayLayout->addWidget(successorDisplay, 0, 0);
    successorGroup->setLayout(successorDisplayLayout);


    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(keyValEnterGroup, 0, 0);
    layout->addWidget(nodeEnterGroup, 1, 0);
    layout->addWidget(keySearchGroup, 2, 0);
    layout->addWidget(successorGroup, 0, 1);

    setLayout(layout);
}

void DHTServer::bindNetSocket(NetSocket *ns) {
    netSocket = ns;

    QHostInfo info = QHostInfo::fromName(QHostInfo::localHostName());
    QString ip = info.addresses().first().toString();

    localOrigin = QString("%1:%2").arg(ip).arg(netSocket->bindedPort);

    QByteArray byteArray;
    byteArray.append(localOrigin.toUtf8());
    QByteArray h = QCA::Hash("md5").hash(byteArray).toByteArray();
    QString md5re = h.toHex();
    hashId = Util::construct32bitsHashId(md5re);

    bool ok;
    serverId = hashId.toUInt(&ok,16);
    qDebug() << localOrigin << " MD5'ed into HashId: " << hashId << " serverId: " << serverId;

    connect(netSocket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
    setWindowTitle(localOrigin);
}

void DHTServer::sendMessage(QVariantMap m, QHostAddress ip, quint16 prt) {
    QByteArray data;
    QDataStream *stream = new QDataStream(&data, QIODevice::WriteOnly);
    (*stream) << m;
    netSocket->writeDatagram(data.data(), data.size(), ip, prt);
}

void DHTServer::receiveMessage() {
    while (netSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(netSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        netSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        
        QVariantMap receivedMessageMap;
        QDataStream *stream = new QDataStream(&datagram, QIODevice::ReadOnly);
        (*stream) >> receivedMessageMap;
        delete stream;

        qDebug() << receivedMessageMap;

        if (receivedMessageMap.contains("JoinRequest")) {
            qDebug() << "I got a join request";

            quint64 fromId= receivedMessageMap["ServerId"].toUInt();

            if (successors.isEmpty() ||
                (fromId > serverId && fromId < successors[0]["ServerId"].toUInt()) ||
                (fromId > serverId && fromId > successors[0]["ServerId"].toUInt() && serverId < successors[0]["ServerId"].toUInt())) {

                /* Joining DHTServer has found the right place. */
                QVariantMap successor;
                successor["Origin"] = receivedMessageMap["Origin"];
                successor["HashId"] = receivedMessageMap["HashId"];
                successor["ServerId"] = receivedMessageMap["ServerId"];
                successors.append(successor);
            } else {
                /* Not the right place, forwarding the join request to its successor. */
                QString successorOrigin = successors[0]["Origin"].toString();
                QStringList list = successorOrigin.split(":");
                sendMessage(receivedMessageMap, QHostAddress(list[0]), list[1].toInt());
            }

        }
    }

}


void DHTServer::nodeJoinBtnClickedHandler() {
    QHostInfo info = QHostInfo::fromName(QHostInfo::localHostName());
    qDebug() <<"IP Address: "<<info.addresses();

    QString toAdd = nodeEnterInput->text();
    QStringList list = toAdd.split(":");

    if (list.length() < 2) {
        return;
    }

    QString host = list[0];
    quint16 port = list[1].toInt();
    hostHunter = new HostNameLookup(port);

    QHostInfo::lookupHost(host, this, SLOT(lookedupHandler(QHostInfo)));
    nodeEnterInput->clear();
}

void DHTServer::keyValInsertionHandler() {
    QString key = keyInsertInput->text().simplified();
    QString val = valInsertInput->text().simplified();

    if (key == "" || val == "") {
        return;
    }

    QString info =  QString("Insert key: val => %1: %2").arg(key).arg(val);
    qDebug() << info;
}

void DHTServer::lookedupHandler(const QHostInfo &host) {
    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        return;
    }

    if (host.error() == QHostInfo::HostNotFound) {
        qDebug() << "Host Not Found:";
        return;
    }

    QString ip = host.addresses().first().toString();
    qDebug() << "Found address:" << ip;
    hostHunter->ipAddr = ip;

    QVariantMap joinMessage;
    joinMessage["JoinRequest"] = true;
    joinMessage["Origin"] = localOrigin;
    joinMessage["HashId"] = hashId;
    joinMessage["ServerId"] = serverId;
    sendMessage(joinMessage, QHostAddress(hostHunter->ipAddr), hostHunter->port);

    /*
    QVariantMap peer;

    peer["ipAddr"] = hostHunter->ipAddr;
    peer["port"] = hostHunter->port;
    peerList.append(peer);

    QString peerInfo = QString("%1 %2").arg(hostHunter->ipAddr).arg(hostHunter->port);
    peersInfoDisplay->append(peerInfo);
    */
}





DHTServer::~DHTServer()
{
    delete ui;
}
