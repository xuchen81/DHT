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
    successorDisplay->setFixedHeight(80);
    successorDisplay->setFixedWidth(200);
    QGridLayout *successorDisplayLayout = new QGridLayout;
    successorDisplayLayout->addWidget(successorDisplay, 0, 0);
    successorGroup->setLayout(successorDisplayLayout);

    QGroupBox *predecessorGroup = new QGroupBox(tr("Predecessors"));
    predecessorDisplay = new QTextEdit(this);
    predecessorDisplay->setReadOnly(true);
    predecessorDisplay->setFixedHeight(80);
    predecessorDisplay->setFixedWidth(200);
    QGridLayout *predecessorDisplayLayout = new QGridLayout;
    predecessorDisplayLayout->addWidget(predecessorDisplay, 0, 0);
    predecessorGroup->setLayout(predecessorDisplayLayout);


    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(keyValEnterGroup, 0, 0);
    layout->addWidget(nodeEnterGroup, 1, 0);
    layout->addWidget(keySearchGroup, 2, 0);
    layout->addWidget(successorGroup, 0, 1);
    layout->addWidget(predecessorGroup, 1, 1);

    setLayout(layout);
}

void DHTServer::bindNetSocket(NetSocket *ns) {
    netSocket = ns;

    QHostInfo info = QHostInfo::fromName(QHostInfo::localHostName());
    QString ip = info.addresses().first().toString();

    localOrigin = QString("%1:%2").arg(ip).arg(netSocket->bindedPort);
    hashId = Util::getHashId(localOrigin);

    bool ok;
    serverId = hashId.toUInt(&ok,16);
    qDebug() << localOrigin << " MD5'ed into HashId: " << hashId << " serverId: " << serverId;

    connect(netSocket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
    setWindowTitle(localOrigin);
}

void DHTServer::normalLeave() {
    if (!successors.isEmpty()) {
        QStringList slist = successors[0]["Origin"].toString().split(":");

        if (!predecessors.isEmpty()) {
            QStringList plist = predecessors[0]["Origin"].toString().split(":");

            if (successors[0]["ServerId"].toUInt() == predecessors[0]["ServerId"].toUInt()) {
                // Only two DHT servers are in the chrod, and 1 wants to leave.
                QVariantMap leaveMess;
                leaveMess["NodeExit"] = true;
                leaveMess["UpdateNeighbsToEmpty"] = true;
                sendMessage(leaveMess, QHostAddress(slist[0]), slist[1].toUInt());
            } else {
                // At least three DHT servers are in the chrod, and 1 wants to leave.
                QVariantMap leaveMessToSucc;
                leaveMessToSucc["UpdatePredRequest"] = true;
                leaveMessToSucc["Origin"] = predecessors[0]["Origin"];
                leaveMessToSucc["HashId"] = predecessors[0]["HashId"];
                leaveMessToSucc["ServerId"] = predecessors[0]["ServerId"];
                sendMessage(leaveMessToSucc, QHostAddress(slist[0]), slist[1].toUInt());

                QVariantMap leaveMessToPred;
                leaveMessToPred["UpdateSuccRequest"] = true;
                leaveMessToPred["Origin"] = successors[0]["Origin"];
                leaveMessToPred["HashId"] = successors[0]["HashId"];
                leaveMessToPred["ServerId"] = successors[0]["ServerId"];
                sendMessage(leaveMessToPred, QHostAddress(plist[0]), plist[1].toUInt());
            }
        }
    }
}

void DHTServer::closeEvent(QCloseEvent *event) {
    QMessageBox msgBox;
    msgBox.setText("Are you sure you want to close this DHT server?");
    msgBox.setStandardButtons(QMessageBox::Close | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Close);
    int result = msgBox.exec();
    switch (result) {
      case QMessageBox::Close:
          qDebug() << "This DHT Server is closed...";
          normalLeave();
          event->accept();
          break;
      case QMessageBox::Cancel:
          event->ignore();
          break;
      default:
          QDialog::closeEvent(event);
          break;
    }
}

void DHTServer::updateSuccessor(QVariantMap succ) {
    successors.clear();
    successors.append(succ);
    successorDisplay->clear();
    successorDisplay->append(QString("Origin: %1").arg(succ["Origin"].toString()));
    successorDisplay->append(QString("HashId: %1").arg(succ["Origin"].toString()));
    successorDisplay->append(QString("ServerId: %1").arg(succ["ServerId"].toString()));
}

void DHTServer::updatePredecessor(QVariantMap pred) {
    predecessors.clear();
    predecessors.append(pred);
    predecessorDisplay->clear();
    predecessorDisplay->append(QString("Origin: %1").arg(pred["Origin"].toString()));
    predecessorDisplay->append(QString("HashId: %1").arg(pred["Origin"].toString()));
    predecessorDisplay->append(QString("ServerId: %1").arg(pred["ServerId"].toString()));
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

        if (receivedMessageMap.contains("JoinRequest")) {
            quint64 fromId = receivedMessageMap["ServerId"].toUInt();

            if (successors.isEmpty() ||
                (fromId > serverId && fromId < successors[0]["ServerId"].toUInt()) ||
                (fromId > serverId && fromId > successors[0]["ServerId"].toUInt() && serverId > successors[0]["ServerId"].toUInt()) ||
                (fromId < serverId && fromId < successors[0]["ServerId"].toUInt() && serverId > successors[0]["ServerId"].toUInt())) {

                if (successors.isEmpty()) {
                    qDebug() << "============================================== = = = = case 1";
                } else if (fromId > serverId && fromId < successors[0]["ServerId"].toUInt()) {
                    qDebug() << "============================================== = = = = case 2";
                } else if (fromId > serverId && fromId > successors[0]["ServerId"].toUInt() && serverId > successors[0]["ServerId"].toUInt()) {
                    qDebug() << "============================================== = = = = case 3";
                } else if (fromId < serverId && fromId < successors[0]["ServerId"].toUInt() && serverId > successors[0]["ServerId"].toUInt()) {
                    qDebug() << "============================================== = = = = case 4";
                }

                QVariantMap joinAcceptedMessage;
                joinAcceptedMessage["JoinRequestAccepted"] = true;

                QVariantMap pred;
                pred["Origin"] = localOrigin;
                pred["HashId"] = hashId;
                pred["ServerId"] = serverId;

                joinAcceptedMessage["Pred"] = pred;

                if (!successors.isEmpty()) {
                    QVariantMap succ;
                    succ["Origin"] = successors[0]["Origin"];
                    succ["HashId"] = successors[0]["HashId"];
                    succ["ServerId"] = successors[0]["ServerId"];
                    joinAcceptedMessage["Succ"] = succ;
                }
                QStringList jlist = receivedMessageMap["Origin"].toString().split(":");
                sendMessage(joinAcceptedMessage, QHostAddress(jlist[0]), jlist[1].toUInt());
            } else {
                qDebug() << "Forwarding join request ! " << endl;
                QString successorOrigin = successors[0]["Origin"].toString();
                QStringList list = successorOrigin.split(":");
                sendMessage(receivedMessageMap, QHostAddress(list[0]), list[1].toUInt());
            }
        } else if (receivedMessageMap.contains("JoinRequestAccepted")) {
            QVariantMap pred = receivedMessageMap["Pred"].toMap();
            updatePredecessor(pred);

            if (receivedMessageMap.contains("Succ")) {
                QVariantMap succ = receivedMessageMap["Succ"].toMap();
                updateSuccessor(succ);
            } else {
                // Only 1 DHT server before the join
                // In this case, pred will also be the succ of the joining DHT server
                updateSuccessor(pred);
            }

            // send update request to both succ and pred
            QVariantMap updateSuccMessage;
            updateSuccMessage["UpdateSuccRequest"] = true;
            updateSuccMessage["Origin"] = localOrigin;
            updateSuccMessage["HashId"] = hashId;
            updateSuccMessage["ServerId"] = serverId;

            QStringList plist = predecessors[0]["Origin"].toString().split(":");
            sendMessage(updateSuccMessage, QHostAddress(plist[0]), plist[1].toUInt());

            QVariantMap updatePredMessage;
            updatePredMessage["UpdatePredRequest"] = true;
            updatePredMessage["Origin"] = localOrigin;
            updatePredMessage["HashId"] = hashId;
            updatePredMessage["ServerId"] = serverId;

            QStringList slist = successors[0]["Origin"].toString().split(":");
            sendMessage(updatePredMessage, QHostAddress(slist[0]), slist[1].toUInt());

        } else if (receivedMessageMap.contains("UpdateSuccRequest")) {
            QVariantMap succ;
            succ["Origin"] = receivedMessageMap["Origin"];
            succ["HashId"] = receivedMessageMap["HashId"];
            succ["ServerId"] = receivedMessageMap["ServerId"];

            updateSuccessor(succ);
        } else if (receivedMessageMap.contains("UpdatePredRequest")) {
            QVariantMap pred;
            pred["Origin"] = receivedMessageMap["Origin"];
            pred["HashId"] = receivedMessageMap["HashId"];
            pred["ServerId"] = receivedMessageMap["ServerId"];

            updatePredecessor(pred);
        } else if (receivedMessageMap.contains("NodeExit") && receivedMessageMap.contains("UpdateNeighbsToEmpty")) {
            successors.clear();
            successorDisplay->clear();
            predecessors.clear();
            predecessorDisplay->clear();
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
    quint16 port = list[1].toUInt();
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
    QString keyHash = Util::getHashId(key);

    bool ok;
    quint64 keyId = keyHash.toUInt(&ok,16);

    qDebug() << info;
    qDebug() << "key hash: " << keyHash << " key id: " << keyId;

    if (predecessors.isEmpty() || (keyId < serverId && keyId > predecessors[0]["ServerId"].toUInt())) {
        QVariantMap kvPair;
        kvPair["Key"] = key;
        kvPair["KeyHashId"] = keyHash;
        kvPair["KeyId"] = keyId;
        kvPair["Val"] = val;
        kvs.insert(keyId, kvPair);
        qDebug() << kvs;
    } else {
        /* Forwarding the key insertion request. */
        QVariantMap kvInsertRequest;
        kvInsertRequest["KVInsertRequest"] = true;
        kvInsertRequest["Key"] = key;
        kvInsertRequest["KeyHashId"] = keyHash;
        kvInsertRequest["KeyId"] = keyId;
        kvInsertRequest["Key"] = val;
    }
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
