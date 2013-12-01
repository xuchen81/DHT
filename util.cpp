#include "util.h"

Util::Util()
{
}

QString Util::getHashId(QString k) {
    QByteArray byteArray;
    byteArray.append(k.toUtf8());
    QByteArray h = QCA::Hash("md5").hash(byteArray).toByteArray();
    QString md5re = h.toHex();

    srand(time(NULL));
    QString bit32Hash = "";
    for (int i = 0; i < 8; i ++) {
        int r = qrand() % 32;
        bit32Hash.append(md5re.at(r));
    }

    return bit32Hash;
}