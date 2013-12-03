#include "util.h"

Util::Util()
{
}

QString Util::getHashId(QString k) {
    QByteArray byteArray;
    byteArray.append(k.toUtf8());
    QByteArray h = QCA::Hash("md5").hash(byteArray).toByteArray();
    QString md5re = h.toHex();

    QString bit32Hash = "";
    bit32Hash.append(md5re.at(29));
    bit32Hash.append(md5re.at(23));
    bit32Hash.append(md5re.at(8));
    bit32Hash.append(md5re.at(25));
    bit32Hash.append(md5re.at(27));
    bit32Hash.append(md5re.at(11));
    bit32Hash.append(md5re.at(0));
    bit32Hash.append(md5re.at(5));

    /* Stopped randomizing everytime, because searching a key will give key different ID
    srand(time(NULL));
    for (int i = 0; i < 8; i ++) {
        int r = qrand() % 32;
        qDebug() << "random: " << r;
        bit32Hash.append(md5re.at(r));
    }
    */
    return bit32Hash;
}