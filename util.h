#ifndef UTIL_H
#define UTIL_H

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
#include <QFileDialog>
#include <QtCrypto>


class Util
{
public:
    Util();
    static QString getHashId(QString key);
};

#endif // UTIL_H
