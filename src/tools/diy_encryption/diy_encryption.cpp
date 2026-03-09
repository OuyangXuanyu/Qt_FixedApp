//
// Created by Administrator on 26-1-14.
//

#include "diy_encryption.h"

QString hashPasswordQt(const QString& password)
{
    const QByteArray bytes = password.toUtf8();  // 等价 password.encode()
    const QByteArray hash = QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);

    return hash.toHex();
}
