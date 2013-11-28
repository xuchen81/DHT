#include "util.h"

Util::Util()
{
}

QString Util::construct32bitsHashId(QString md5HashResult) {
    srand(time(NULL));
    QString bit32Hash = "";
    for (int i = 0; i < 8; i ++) {
        int r = qrand() % 32;
        bit32Hash.append(md5HashResult.at(r));
    }

    return bit32Hash;
}
