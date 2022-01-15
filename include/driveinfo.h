// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QMetaType>
#include <QUuid>
#include <QVector>

class DriveInfo
{
    static QVector<DriveInfo> all;

public:
    enum class SIGNATURE : quint8
    {
        NONE = 0x00,
        MBR = 0x01,
        GUID = 0x02,
    };

public:
    QString name = "";
    quint32 partition = 0;
    SIGNATURE signature_type = SIGNATURE::NONE;
    QUuid signature = {};
    quint64 start = 0;
    quint64 size = 0;

public:
    static QVector<DriveInfo> get_all(bool refresh = false);

    bool operator<(const DriveInfo &info) { return name < info.name; }
};

Q_DECLARE_METATYPE(DriveInfo)
