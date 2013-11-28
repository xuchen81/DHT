#include "netsocket.h"
#include <QUdpSocket>

NetSocket::NetSocket()
{

}

bool NetSocket::bind()
{
    bindedPort = 32768 + (getuid() % 4096)*4;
    if (QUdpSocket::bind(bindedPort)) {
        qDebug() << "bound to UDP port " << bindedPort;
        return true;
    }

    qDebug() << "Binding failed: " << bindedPort;
    return false;
}