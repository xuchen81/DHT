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
    QMenuBar *toolBar = new QMenuBar(this);
    QMenu *menuInfo = new QMenu("&Info");

    QAction *DHTOpen = new QAction("&DHT Info", this);
    menuInfo->addAction(DHTOpen);
    connect(DHTOpen, SIGNAL(triggered()), this, SLOT(displayThisDHT()));

    QAction *neighboursOpen = new QAction("&Successor and Predecessor", this);
    menuInfo->addAction(neighboursOpen);
    connect(neighboursOpen, SIGNAL(triggered()), this, SLOT(neighboursOpenHandler()));

    QAction *keysOpen = new QAction("&Keys Info", this);
    menuInfo->addAction(keysOpen);
    connect(keysOpen, SIGNAL(triggered()), this, SLOT(keysOpenHandler()));

    QAction *ftOpen = new QAction("&Finger Table", this);
    menuInfo->addAction(ftOpen);
    connect(ftOpen, SIGNAL(triggered()), this, SLOT(ftOpenHandler()));

    QAction *cacheOpen = new QAction("&Key Cache", this);
    menuInfo->addAction(cacheOpen);
    connect(cacheOpen, SIGNAL(triggered()), this, SLOT(kvCacheOpenHandler()));

    toolBar->addMenu(menuInfo);

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
    connect(keySearchBtn, SIGNAL(clicked()), this, SLOT(searchKeyBtnClickedHandler()));
    keyDeleteBtn = new QPushButton("Delete");
    connect(keyDeleteBtn, SIGNAL(clicked()), this, SLOT(deleteKeyBtnClickedHandler()));

    QGridLayout *keySearchLayout = new QGridLayout;
    keySearchLayout->addWidget(keySearchLabel, 0, 0);
    keySearchLayout->addWidget(keySearchInput, 0, 1);
    keySearchLayout->addWidget(valFoundLabel, 1, 0);
    keySearchLayout->addWidget(valDisplay, 1, 1);
    keySearchLayout->addWidget(keySearchBtn, 2, 0, 1, 2);
    keySearchLayout->addWidget(keyDeleteBtn,3,0,1,2);
    keySearchGroup->setLayout(keySearchLayout);

    QGroupBox *infoDisplayGroup = new QGroupBox(tr("Info Display"));
    infoDisplay = new QTextEdit(this);
    infoDisplay->setReadOnly(true);
    QGridLayout *successorDisplayLayout = new QGridLayout;
    successorDisplayLayout->addWidget(infoDisplay, 0, 0);
    infoDisplayGroup->setLayout(successorDisplayLayout);

    QGridLayout *layout = new QGridLayout(this);
    layout->setMenuBar(toolBar);
    layout->addWidget(keyValEnterGroup, 0, 0);
    layout->addWidget(nodeEnterGroup, 1, 0);
    layout->addWidget(keySearchGroup, 2, 0);
    layout->addWidget(infoDisplayGroup, 0, 1, 3, 1);

    setLayout(layout);
}

void DHTServer::displayThisDHT() {
    infoDisplay->clear();
    infoDisplay->append("***********************************");
    infoDisplay->append(QString("*    About this DHT:"));
    infoDisplay->append(QString("*    Origin: %1").arg(localOrigin));
    infoDisplay->append(QString("*    HashId: %1").arg(hashId));
    infoDisplay->append(QString("*    ServerId: %1").arg(serverId));
    infoDisplay->append("***********************************");
}

void DHTServer::keysOpenHandler() {
    displayThisDHT();
    for (QHash<quint64,QVariantMap>::iterator i = kvs.begin(); i != kvs.end(); i++) {
        infoDisplay->append(QString("KeyHashId: %1").arg(i.value()["KeyHashId"].toString()));
        infoDisplay->append(QString("KeyId: %1").arg(i.value()["KeyId"].toUInt()));
        infoDisplay->append(QString("Key => Val: %1 => %2").arg(i.value()["Key"].toString()).arg(i.value()["Val"].toString()));
        infoDisplay->append("*****************************");
    }
}

void DHTServer::ftOpenHandler() {
    displayThisDHT();
    for (QHash<quint64,QVariantMap>::iterator i = fingerTable.begin(); i != fingerTable.end(); i++) {
        infoDisplay->append(QString("Key: %1").arg(i.key()));
        infoDisplay->append(QString("Origin: %1").arg(i.value()["Origin"].toString()));
        infoDisplay->append(QString("HashId: %1").arg(i.value()["HashId"].toString()));
        infoDisplay->append(QString("ServerId: %1").arg(i.value()["ServerId"].toString()));
        infoDisplay->append(QString("Distance: %1").arg(i.value()["Distance"].toUInt()));
        infoDisplay->append(QString("HopPoint: %1").arg(i.value()["HopPoint"].toUInt()));
        infoDisplay->append(QString("*****************"));
    }
}

void DHTServer::kvCacheOpenHandler() {
    displayThisDHT();
    for (QHash<quint64,QVariantMap>::iterator i = kvCache.begin(); i != kvCache.end(); i++) {
        infoDisplay->append(QString("Key: %1").arg(i.value()["Key"].toString()));
        infoDisplay->append(QString("KeyId: %1").arg(i.value()["KeyId"].toUInt()));
        infoDisplay->append(QString("Origin: %1").arg(i.value()["Origin"].toString()));
        infoDisplay->append(QString("**************************"));
    }
}

void DHTServer::neighboursOpenHandler() {
    displayThisDHT();
    infoDisplay->append(QString("-----------------------------------------------"));
    if (!successors.isEmpty()) {
        infoDisplay->append(QString("|    Successor:"));
        infoDisplay->append(QString("|    Origin: %1").arg(successors[0]["Origin"].toString()));
        infoDisplay->append(QString("|    HashId: %1").arg(successors[0]["HashId"].toString()));
        infoDisplay->append(QString("|    ServerId: %1").arg(successors[0]["ServerId"].toString()));
    } else {
        infoDisplay->append("There is no successor at this moment.");
    }
    infoDisplay->append(QString("-----------------------------------------------"));
    if (!predecessors.isEmpty()) {
        infoDisplay->append(QString("|    Predecessor:"));
        infoDisplay->append(QString("|    Origin: %1").arg(predecessors[0]["Origin"].toString()));
        infoDisplay->append(QString("|    HashId: %1").arg(predecessors[0]["HashId"].toString()));
        infoDisplay->append(QString("|    ServerId: %1").arg(predecessors[0]["ServerId"].toString()));
    } else {
        infoDisplay->append("There is no predecessor at this moment.");
    }
    infoDisplay->append(QString("-----------------------------------------------"));
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
    setWindowFlags(Qt::Dialog | Qt::Desktop);
    initFingerTable();
    displayThisDHT();
}

void DHTServer::initFingerTable() {
    //hopPoint is the hash value to hit
    //Distance is the clockwise amount from the hop point to its master node

    QVariantMap node;
    node["Origin"] = localOrigin;
    node["ServerId"] = serverId;
    node["HashId"] = hashId;
    quint64 exp2 = 1;
    quint64 distance;
    quint64 hopPoint;

    for (int i=0;i<32;i++) {
        hopPoint = ((exp2 << i) + serverId) > CHORD_RANGE ? (((exp2 << i) + serverId) - CHORD_RANGE) : ((exp2 << i) + serverId);

        node["HopPoint"] = hopPoint;
        distance = (serverId - hopPoint) > 0 ? (serverId - hopPoint) : (CHORD_RANGE - hopPoint + serverId);
        node["Distance"] = distance;
        fingerTable.insert(i, node);
    }
}

void DHTServer::spreadKeysToNeighbours() {
    if (successors.isEmpty() || predecessors.isEmpty()) return;

    for (QHash<quint64,QVariantMap>::iterator i = kvs.begin(); i != kvs.end(); i++) {
        QVariantMap keyValMigration;
        keyValMigration["KVInsertRequest"] = true;
        keyValMigration["Key"] = i.value()["Key"];
        keyValMigration["KeyHashId"] = i.value()["KeyHashId"];
        keyValMigration["KeyId"] = i.value()["KeyId"];
        keyValMigration["Val"] = i.value()["Val"];

        QStringList slist = successors[0]["Origin"].toString().split(":");
        sendMessage(keyValMigration, QHostAddress(slist[0]), slist[1].toUInt());
    }
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

            spreadKeysToNeighbours();

            QVariantMap leaveMessFinger;
            leaveMessFinger["UpdateFTNodeLeave"] = true;
            leaveMessFinger["Origin"] = localOrigin;
            leaveMessFinger["HashId"] = hashId;
            leaveMessFinger["ServerId"] = serverId;
            leaveMessFinger["Succ"] = successors[0];
            sendMessage(leaveMessFinger,QHostAddress(slist[0]),slist[1].toUInt());
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

void DHTServer::updateFingerTable(QVariantMap node) {
    if (node["Direction"] == "join") {
        quint64 distance;

        QVariantMap hopSucc;

        for (int i=0;i<32;i++) {
            hopSucc = fingerTable[i];
            distance = (node["ServerId"].toUInt() - hopSucc["HopPoint"].toUInt()) > 0 ?
                       (node["ServerId"].toUInt() - hopSucc["HopPoint"].toUInt()):
                       (CHORD_RANGE + node["ServerId"].toUInt() - hopSucc["HopPoint"].toUInt());

            if (i==31) {
                qDebug() << distance << " : " << hopSucc["Distance"].toUInt()<<endl;
                qDebug() << hopSucc << endl;
            }

            if (hopSucc["Distance"].toUInt()>distance){
                if (i==31) {
                    qDebug()<<hopSucc["Distance"].toUInt()<<"****"<<distance<<endl;
                }

                hopSucc["Origin"] = node["Origin"];
                hopSucc["ServerId"] = node["ServerId"];
                hopSucc["HashId"] = node["HashId"];
                hopSucc["Distance"] = distance;
                fingerTable[i] = hopSucc;
            }
        }
    } else if (node["Direction"] == "exit") {
        QVariantMap hopSucc;
        QVariantMap leaveSucc = node["Succ"].toMap();
        quint64 distance;

        for (int i=0;i<32;i++) {
            hopSucc = fingerTable[i];
            if (hopSucc["Origin"] == node["Origin"]) {
                distance = (leaveSucc["ServerId"].toUInt() - hopSucc["HopPoint"].toUInt()) > 0  ?
                           (leaveSucc["ServerId"].toUInt() - hopSucc["HopPoint"].toUInt()) :
                           (CHORD_RANGE + leaveSucc["ServerId"].toUInt() - hopSucc["HopPoint"].toUInt());

                hopSucc["Origin"] = leaveSucc["Origin"];
                hopSucc["ServerId"] = leaveSucc["ServerId"];
                hopSucc["HashId"] = node["HashId"];
                hopSucc["Distance"] = distance;
                fingerTable[i] = hopSucc;
            }
        }
    }
}

void DHTServer::updateSuccessor(QVariantMap succ) {
    successors.clear();
    successors.append(succ);
    neighboursOpenHandler();
}

void DHTServer::updatePredecessor(QVariantMap pred) {
    predecessors.clear();
    predecessors.append(pred);
    neighboursOpenHandler();
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
                (fromId > serverId && fromId > successors[0]["ServerId"].toUInt() && serverId > successors[0]["ServerId"].toUInt()) || // max
                (fromId < serverId && fromId < successors[0]["ServerId"].toUInt() && serverId > successors[0]["ServerId"].toUInt())) { // min

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

            /* Migrate keys. */
            QVariantMap migrateKeysMess;
            migrateKeysMess["MigrateKeys"] = true;
            migrateKeysMess["Origin"] = localOrigin;
            migrateKeysMess["HashId"] = hashId;
            migrateKeysMess["ServerId"] = serverId;
            sendMessage(migrateKeysMess, QHostAddress(slist[0]), slist[1].toUInt());

            //send fingertable update info to succ
            QVariantMap updateFingerTabMessage;
            updateFingerTabMessage["UpdateFinMessage"] = true;
            updateFingerTabMessage["Direction"] = "join";
            updateFingerTabMessage["Origin"] = localOrigin;
            updateFingerTabMessage["HashId"] = hashId;
            updateFingerTabMessage["ServerId"] = serverId;

            QStringList flist = successors[0]["Origin"].toString().split(":");
            sendMessage(updateFingerTabMessage, QHostAddress(flist[0]), flist[1].toUInt());

            qDebug()<<"***********JoinRequestAccepted**********"<<endl;

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
        } else if (receivedMessageMap.contains("UpdateFinMessage") && (receivedMessageMap["Direction"]== "join") && 
                   localOrigin != receivedMessageMap["Origin"]) {

            QVariantMap node;
            node["Origin"] = receivedMessageMap["Origin"];
            node["HashId"] = receivedMessageMap["HashId"];
            node["ServerId"] = receivedMessageMap["ServerId"];
            node["Direction"] = receivedMessageMap["Direction"];

            updateFingerTable(node);

            qDebug()<<"***********UpdateFinMessage**********"<<endl;

            QStringList flist = successors[0]["Origin"].toString().split(":");
            sendMessage(receivedMessageMap,QHostAddress(flist[0]),flist[1].toUInt());

            // send to new node localOrigin info for updating finger table of new node
            QVariantMap updateFinNewnodeMessage;
            updateFinNewnodeMessage["UpdateNewFinMessage"] = true;
            updateFinNewnodeMessage["Origin"] = localOrigin;
            updateFinNewnodeMessage["HashId"] = hashId;
            updateFinNewnodeMessage["ServerId"] = serverId;
            updateFinNewnodeMessage["Direction"] = "join";

            QStringList newNodelist = node["Origin"].toString().split(":");
            sendMessage(updateFinNewnodeMessage,QHostAddress(newNodelist[0]),newNodelist[1].toUInt());
        } else if (receivedMessageMap.contains("UpdateNewFinMessage")) {
            QVariantMap node;
            node["Origin"] = receivedMessageMap["Origin"];
            node["HashId"] = receivedMessageMap["HashId"];
            node["ServerId"] = receivedMessageMap["ServerId"];
            node["Direction"] = receivedMessageMap["Direction"];
            qDebug()<<"***********UpdateNewFinMessage**********"<<endl;
            updateFingerTable(node);
        } else if (receivedMessageMap.contains("UpdateFTNodeLeave")) {
            QVariantMap node;
            node["Origin"] = receivedMessageMap["Origin"];
            node["HashId"] = receivedMessageMap["HashId"];
            node["ServerId"] = receivedMessageMap["ServerId"];
            node["Direction"] = "exit";
            node["Succ"] = receivedMessageMap["Succ"];

            updateFingerTable(node);
            if (!successors.isEmpty()&& successors[0]["Origin"].toString() != node["Succ"].toMap()["Origin"].toString()) {
                QStringList flist = successors[0]["Origin"].toString().split(":");
                sendMessage(receivedMessageMap,QHostAddress(flist[0]), flist[1].toUInt());
            }
        } else if (receivedMessageMap.contains("NodeExit") && receivedMessageMap.contains("UpdateNeighbsToEmpty")) {
            successors.clear();
            predecessors.clear();
        } else if (receivedMessageMap.contains("KVInsertRequest")) {

            quint64 keyId = receivedMessageMap["KeyId"].toUInt();

            qDebug() << "I got this Key insert request";
            if (successors.isEmpty() ||
                (keyId < serverId && keyId > predecessors[0]["ServerId"].toUInt()) ||
                (keyId < serverId && keyId < predecessors[0]["ServerId"].toUInt() && serverId < predecessors[0]["ServerId"].toUInt()) ||  // min
                (keyId > serverId && keyId > predecessors[0]["ServerId"].toUInt() && serverId < predecessors[0]["ServerId"].toUInt())) {  // max)

                qDebug() << "I accepted";
                QVariantMap kvPair;
                kvPair["Key"] = receivedMessageMap["Key"];
                kvPair["KeyHashId"] = receivedMessageMap["KeyHashId"];
                kvPair["KeyId"] = receivedMessageMap["KeyId"];
                kvPair["Val"] = receivedMessageMap["Val"];

                kvs.insert(keyId, kvPair);
                keysOpenHandler();
            } else { /* Forwarding the key insertion request. */
                QStringList plist = predecessors[0]["Origin"].toString().split(":");
                sendMessage(receivedMessageMap, QHostAddress(plist[0]), plist[1].toUInt());
            }
        } else if (receivedMessageMap.contains("MigrateKeys")) {
            QStringList l = receivedMessageMap["Origin"].toString().split(":");
            QList<quint64> toRemove;

            for (QHash<quint64,QVariantMap>::iterator i = kvs.begin(); i != kvs.end(); i++) {
                if (serverId < receivedMessageMap["ServerId"].toUInt()) {
                    if (i.key() < receivedMessageMap["ServerId"].toUInt() && i.key() > serverId) {
                        QVariantMap keyValMigration;
                        keyValMigration["KVInsertRequest"] = true;
                        keyValMigration["Key"] = i.value()["Key"];
                        keyValMigration["KeyHashId"] = i.value()["KeyHashId"];
                        keyValMigration["KeyId"] = i.value()["KeyId"];
                        keyValMigration["Val"] = i.value()["Val"];

                        sendMessage(keyValMigration, QHostAddress(l[0]), l[1].toUInt());
                        toRemove.append(i.key());
                    }
                } else {
                    if (i.key() < receivedMessageMap["ServerId"].toUInt()) {

                        QVariantMap keyValMigration;
                        keyValMigration["KVInsertRequest"] = true;
                        keyValMigration["Key"] = i.value()["Key"];
                        keyValMigration["KeyHashId"] = i.value()["KeyHashId"];
                        keyValMigration["KeyId"] = i.value()["KeyId"];
                        keyValMigration["Val"] = i.value()["Val"];

                        sendMessage(keyValMigration, QHostAddress(l[0]), l[1].toUInt());
                        toRemove.append(i.key());
                    }
                }
            }

            for (int i = 0; i < toRemove.length(); i++) {
                kvs.remove(toRemove[i]);
            }
        } else if (receivedMessageMap.contains("KVSearchRequest")) {
            quint64 keyId = receivedMessageMap["KeyId"].toUInt();
            if (kvs.find(keyId)!= kvs.end()){
                QVariantMap kvFoundMessage;
                kvFoundMessage["FoundKV"] = true;
                kvFoundMessage["KeyId"] = keyId;
                kvFoundMessage["Key"] = kvs[keyId]["Key"];
                kvFoundMessage["Val"] = kvs[keyId]["Val"];
                kvFoundMessage["KeyHashId"] = kvs[keyId]["KeyHashId"];
                kvFoundMessage["Origin"] = localOrigin;

                QStringList olist = receivedMessageMap["Origin"].toString().split(":");
                sendMessage(kvFoundMessage,QHostAddress(olist[0]),olist[1].toUInt());
            }else if (receivedMessageMap["HopLimit"].toInt() > 0){
                receivedMessageMap["HopLimit"] = receivedMessageMap["HopLimit"].toInt() - 1;
                QString successorAddr = searchFinTable(keyId);

                QStringList slist = successorAddr.split(":");
                sendMessage(receivedMessageMap, QHostAddress(slist[0]), slist[1].toUInt());
            } else if (receivedMessageMap["HopLimit"].toInt() == 0) {
                valDisplay->clear();
            }
        } else if(receivedMessageMap.contains("KVCacheSearchRequest")) {
            quint64 keyId = receivedMessageMap["KeyId"].toUInt();
            if (kvs.find(keyId)!= kvs.end()){
                QVariantMap kvFoundMessage;
                kvFoundMessage["FoundKV"] = true;
                kvFoundMessage["KeyId"] = keyId;
                kvFoundMessage["Key"] = kvs[keyId]["Key"];
                kvFoundMessage["Val"] = kvs[keyId]["Val"];
                kvFoundMessage["KeyHashId"] = kvs[keyId]["KeyHashId"];
                kvFoundMessage["Origin"] = localOrigin;

                QStringList olist = receivedMessageMap["Origin"].toString().split(":");
                sendMessage(kvFoundMessage, QHostAddress(olist[0]), olist[1].toUInt());
            }
            else {
                QVariantMap kvCacheDeleteMessage;
                kvCacheDeleteMessage["KeyDeleted"] = true;
                kvCacheDeleteMessage["KeyId"] = receivedMessageMap["KeyId"];
                QStringList slist = receivedMessageMap["Origin"].toString().split(":");
                sendMessage(kvCacheDeleteMessage, QHostAddress(slist[0]), slist[1].toUInt());
            }
        } else if (receivedMessageMap.contains("KeyDeleted")){
            quint64 keyId = receivedMessageMap["KeyId"].toUInt();
            kvCache.remove(keyId);
        } else if (receivedMessageMap.contains("KVDeleteRequest")){
            quint64 keyId = receivedMessageMap["KeyId"].toUInt();

            if (kvs.find(keyId)!= kvs.end()){
               kvs.remove(keyId);
               keysOpenHandler();
            }
            else if (receivedMessageMap["HopLimit"].toInt() > 0) {
                receivedMessageMap["HopLimit"] = receivedMessageMap["HopLimit"].toInt() - 1;
                QString successorAddr = searchFinTable(keyId);

                QStringList slist = successorAddr.split(":");
                sendMessage(receivedMessageMap, QHostAddress(slist[0]), slist[1].toUInt());
            }
        } else if (receivedMessageMap.contains("FoundKV")) {
            QString origin = receivedMessageMap["Origin"].toString();
            quint64 keyId = receivedMessageMap["KeyId"].toUInt();
            QString val = receivedMessageMap["Val"].toString();
            valDisplay->clear();
            valDisplay->setText(val);
            QVariantMap kv;
            kv["Origin"] = origin;
            kv["Key"] = receivedMessageMap["Key"].toString();
            kv["KeyId"] = keyId;
            if (kvCache.find(keyId) == kvCache.end()) {
               kvCache.insert(keyId, kv);
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
    quint16 port = list[1].toUInt();
    hostHunter = new HostNameLookup(port);

    QHostInfo::lookupHost(host, this, SLOT(lookedupHandler(QHostInfo)));
    nodeEnterInput->clear();
}

void DHTServer::keyValInsertionHandler() {
    QString key = keyInsertInput->text().simplified();
    QString val = valInsertInput->text().simplified();
    keyInsertInput->clear();
    valInsertInput->clear();

    if (key == "" || val == "") {
        return;
    }

    QString info =  QString("Insert key: val => %1: %2").arg(key).arg(val);
    QString keyHash = Util::getHashId(key);

    bool ok;
    quint64 keyId = keyHash.toUInt(&ok,16);

    qDebug() << info;
    qDebug() << "key hash: " << keyHash << " key id: " << keyId;

    if (predecessors.isEmpty() || 
        (keyId < serverId && keyId > predecessors[0]["ServerId"].toUInt()) ||
        (keyId < serverId && keyId < predecessors[0]["ServerId"].toUInt() && serverId < predecessors[0]["ServerId"].toUInt()) ||  // min
        (keyId > serverId && keyId > predecessors[0]["ServerId"].toUInt() && serverId < predecessors[0]["ServerId"].toUInt())) {  // max
        QVariantMap kvPair;
        kvPair["Key"] = key;
        kvPair["KeyHashId"] = keyHash;
        kvPair["KeyId"] = keyId;
        kvPair["Val"] = val;
        kvs.insert(keyId, kvPair);
        keysOpenHandler();
    } else {
        /* Forwarding the key insertion request. */
        QVariantMap kvInsertRequest;
        kvInsertRequest["KVInsertRequest"] = true;
        kvInsertRequest["Key"] = key;
        kvInsertRequest["KeyHashId"] = keyHash;
        kvInsertRequest["KeyId"] = keyId;
        kvInsertRequest["Val"] = val;

        QStringList plist = predecessors[0]["Origin"].toString().split(":");
        sendMessage(kvInsertRequest, QHostAddress(plist[0]), plist[1].toUInt());
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
}

void DHTServer::searchKeyBtnClickedHandler() {
    QString key = keySearchInput->text();
    QString keyHash = Util::getHashId(key);
    //keySearchInput->clear();
    bool ok;
    quint64 keyId = keyHash.toUInt(&ok,16);
    if (kvs.find(keyId)!=kvs.end()){

       valDisplay->clear();
       valDisplay->setText(kvs[keyId]["Val"].toString());
    }
    else if (kvCache.find(keyId) != kvCache.end()){
        QVariantMap kvSearchMsg;
        kvSearchMsg["KVCacheSearchRequest"] = true;
        kvSearchMsg["KeyId"] = keyId;
        kvSearchMsg["Origin"] = localOrigin;

        QString keyHolder = kvCache[keyId]["Origin"].toString();
        QStringList hlist = keyHolder.split(":");
        sendMessage(kvSearchMsg, QHostAddress(hlist[0]), hlist[1].toUInt());

    }else{
        QVariantMap kvSearchMsg;
        kvSearchMsg["KVSearchRequest"] = true;
        kvSearchMsg["KeyId"] = keyId;
        kvSearchMsg["Origin"] = localOrigin;
        kvSearchMsg["HopLimit"] = HOP_LIMIT;

        QString successorAddr = searchFinTable(keyId);
        QStringList slist = successorAddr.split(":");
        sendMessage(kvSearchMsg, QHostAddress(slist[0]), slist[1].toUInt());
    }
}

void DHTServer::deleteKeyBtnClickedHandler() {
    QString key = keySearchInput->text();
    QString keyHash = Util::getHashId(key);
    keySearchInput->clear();
    valDisplay->clear();
    bool ok;
    quint64 keyId = keyHash.toUInt(&ok,16);
    if (kvs.find(keyId)!=kvs.end()){

       valDisplay->clear();
       valDisplay->setText(kvs[keyId]["Val"].toString());
       kvs.remove(keyId);
       keysOpenHandler();
    }
    else {
        QVariantMap kvDeleteMsg;
        kvDeleteMsg["KVDeleteRequest"] = true;
        kvDeleteMsg["KeyId"] = keyId;
        kvDeleteMsg["Origin"] = localOrigin;
        kvDeleteMsg["HopLimit"] = HOP_LIMIT;

        QString successorAddr = searchFinTable(keyId);
        QStringList slist = successorAddr.split(":");
        sendMessage(kvDeleteMsg, QHostAddress(slist[0]), slist[1].toUInt());
    }
}

QString DHTServer::searchFinTable(quint64 keyId) {
    quint64 currentHop;
    quint64 nextHop;
    QString successorAddr;
    for (int i=0;i<31;i++) {
        currentHop = fingerTable[i]["HopPoint"].toUInt();
        nextHop = fingerTable[i+1]["HopPoint"].toUInt();
        if (keyId>=currentHop && keyId<nextHop) {
            successorAddr = fingerTable[i]["Origin"].toString();
            return successorAddr;
        }
    }
    return fingerTable[31]["Origin"].toString();
}

DHTServer::~DHTServer()
{
    delete ui;
}
