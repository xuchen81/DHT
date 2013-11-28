#ifndef NETSOCKET_H
#define NETSOCKET_H

#include <QUdpSocket>
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
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



class NetSocket : public QUdpSocket
{
    Q_OBJECT
public:
    NetSocket();
    bool bind();
    quint16 bindedPort;
    
signals:
    
public slots:
    
};

#endif // NETSOCKET_H
