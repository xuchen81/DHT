#include "netsocket.h"
#include "dhtserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCA::Initializer qcainit;
    QApplication a(argc, argv);
    DHTServer w;

    NetSocket *sock = new NetSocket();

    if (!sock->bind()) {
        exit(1);
    } else {
        w.bindNetSocket(sock);
    }

    w.show();
    
    return a.exec();
}
