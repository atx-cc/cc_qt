#ifndef CRC_H
#define CRC_H

#include <QByteArray>

quint32 CRC32(const QByteArray &data, int len);
QByteArray intToByteArray(quint32 i);
QByteArray u8ToByteArray(quint8 data);
quint8 besChecksum(const QByteArray &data);

#endif // CRC_H
