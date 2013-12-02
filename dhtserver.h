#ifndef DHTSERVER_H
#define DHTSERVER_H

#include "netsocket.h"
#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QPushButton>
#include <QVariant>
#include <QVariantMap>
#include <QLocalServer>
#include <QHostInfo>
#include <QKeyEvent>
#include <QTimer>
#include <QSignalMapper>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDateTime>
#include <QFileDialog>
#include <QtCrypto>
#include <QIODevice>
#include <QObject>
#include <QLabel>
#include <QGroupBox>
#include <QCloseEvent>
#include <QMessageBox>

namespace Ui {

class DHTServer;
}

class HostNameLookup : public QObject
{
    Q_OBJECT

public:
    HostNameLookup(quint16 p);
    QString ipAddr;
    quint16 port;
};

class DHTServer : public QDialog
{
    Q_OBJECT
    
public:
    explicit DHTServer(QWidget *parent = 0);
    ~DHTServer();
    NetSocket *netSocket;
    void bindNetSocket(NetSocket *netSocket);
    QString localOrigin;
    QString hashId;
    quint64 serverId;


public slots:
    void keyValInsertionHandler();
    void nodeJoinBtnClickedHandler();
    void lookedupHandler(const QHostInfo &host);
    void receiveMessage();
    void sendMessage(QVariantMap m, QHostAddress ip, quint16 p);
    void updateSuccessor(QVariantMap succ);
    void updatePredecessor(QVariantMap pred);

    
private:
    void normalLeave();
    Ui::DHTServer *ui;
    HostNameLookup *hostHunter;

    /* UI elements */
    QLabel *keyValEnterLabel;
    QLineEdit *keyInsertInput;
    QLineEdit *valInsertInput;
    QPushButton *insertBtn;
    QLineEdit *nodeEnterInput;
    QPushButton *nodeJoinBtn;
    QLineEdit *keySearchInput;
    QLineEdit *valDisplay;
    QPushButton *keySearchBtn;
    QTextEdit *successorDisplay;
    QTextEdit *predecessorDisplay;


    QList<QVariantMap> predecessors;
    QList<QVariantMap> successors;

protected:
    void closeEvent(QCloseEvent *event);

};

#endif // DHTSERVER_H
