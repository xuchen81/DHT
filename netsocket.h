#ifndef NETSOCKET_H
#define NETSOCKET_H

#include <QUdpSocket>

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
