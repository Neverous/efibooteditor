// SPDX-License-Identifier: LGPL-3.0-or-later
#include <QDir>

#include "include/compat.h"
#include "include/driveinfo.h"

auto DriveInfo::get_all(bool refresh) -> QVector<DriveInfo>
{
    if(!refresh && !all.empty())
        return all;

    all.clear();

    QDir partuuid("/dev/disk/by-partuuid");
    if(!partuuid.exists())
        return all;

    partuuid.setFilter(QDir::Files);
    for(const auto &part: partuuid.entryInfoList())
    {
        if(!part.isSymLink())
            continue;

        const auto target = QFileInfo{part.symLinkTarget()};

        DriveInfo driveinfo{};
        driveinfo.name = target.fileName();
        if(part.fileName().size() > 11)
        {
            driveinfo.signature_type = DriveInfo::SIGNATURE::GUID;
            driveinfo.signature = QUuid::fromString(part.fileName());
        }

        else
        {
            driveinfo.signature_type = DriveInfo::SIGNATURE::MBR;
            auto parts = part.fileName().split("-");
            uint l = parts[0].toUInt(nullptr, HEX_BASE);
            ushort w1 = parts[1].toUShort(nullptr, HEX_BASE);
            driveinfo.signature = QUuid{l, w1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        }

        const auto &sys_path = QString("/sys/class/block/%1").arg(driveinfo.name);
        {
            QFile file{sys_path + "/partition"};
            if(file.open(QIODevice::ReadOnly))
            {
                driveinfo.partition = file.readAll().toUInt();
            }
        }

        {
            QFile file{sys_path + "/start"};
            if(file.open(QIODevice::ReadOnly))
            {
                driveinfo.start = file.readAll().toULongLong();
            }
        }

        {
            QFile file{sys_path + "/size"};
            if(file.open(QIODevice::ReadOnly))
            {
                driveinfo.size = file.readAll().toULongLong();
            }
        }

        all.append(driveinfo);
    }

    std::sort(std::begin(all), std::end(all));
    return all;
}
