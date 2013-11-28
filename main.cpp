#include "netsocket.h"
#include "dhtserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QCA::Initializer qcainit;
    QApplication a(argc, argv);
    DHTServer w;

    NetSocket *socket = new NetSocket();

    w.show();
    
    return a.exec();
}
