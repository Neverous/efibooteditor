// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "driveinfo.h"

#include <QObject>
#include <Windows.h>
#include <array>

QVector<DriveInfo> DriveInfo::getAll(bool refresh)
{
    if(!refresh && !all.empty())
        return all;

    all.clear();

    std::array<TCHAR, MAX_PATH> volume_name{};
    HANDLE volume_handle = FindFirstVolume(volume_name.data(), static_cast<DWORD>(volume_name.size()));

    if(volume_handle == INVALID_HANDLE_VALUE)
        return all;

    for(BOOL volume_found = true; volume_found; volume_found = FindNextVolume(volume_handle, volume_name.data(), static_cast<DWORD>(volume_name.size())))
    {
        size_t length = _tcsnccnt(volume_name.data(), volume_name.size());
        if(length != 49u)
            continue;

        volume_name[length - 1u] = _T('\0');
        HANDLE device_handle = CreateFile(volume_name.data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        volume_name[length - 1u] = _T('\\');
        if(device_handle == INVALID_HANDLE_VALUE)
            continue;

        STORAGE_DEVICE_NUMBER device_number{};
        if(!DeviceIoControl(device_handle, IOCTL_STORAGE_GET_DEVICE_NUMBER, nullptr, 0, &device_number, sizeof(device_number), nullptr, nullptr))
        {
            CloseHandle(device_handle);
            continue;
        }

        PARTITION_INFORMATION_EX partition_info{};
        if(!DeviceIoControl(device_handle, IOCTL_DISK_GET_PARTITION_INFO_EX, nullptr, 0, &partition_info, sizeof(partition_info), nullptr, nullptr))
        {
            CloseHandle(device_handle);
            continue;
        }

        CloseHandle(device_handle);

        DriveInfo driveinfo{};
        switch(device_number.DeviceType)
        {
        case FILE_DEVICE_CD_ROM:
            driveinfo.name = QObject::tr("CD-ROM %1").arg(device_number.DeviceNumber);
            break;

        case FILE_DEVICE_DISK:
            driveinfo.name = QObject::tr("Disk %1 partition %2").arg(device_number.DeviceNumber).arg(partition_info.PartitionNumber);
            break;

        default:
            driveinfo.name = QObject::tr("Device %1 number %2 partition %3").arg(toHex(device_number.DeviceType)).arg(device_number.DeviceNumber).arg(partition_info.PartitionNumber);
            break;
        }

        QString label{};
        if(std::array<TCHAR, MAX_PATH> volume_label{}; GetVolumeInformation(volume_name.data(), volume_label.data(), static_cast<DWORD>(volume_label.size()), nullptr, nullptr, nullptr, nullptr, 0))
            label = QStringFromTCharArray(volume_label.data());

        switch(partition_info.PartitionStyle)
        {
        case PARTITION_STYLE_GPT:
            driveinfo.signature_type = DriveInfo::SIGNATURE::GUID;
            if(label.isEmpty())
                label = QString::fromWCharArray(partition_info.Gpt.Name);

            driveinfo.signature = partition_info.Gpt.PartitionId;
            break;

        case PARTITION_STYLE_MBR:
            driveinfo.signature_type = DriveInfo::SIGNATURE::MBR;
            driveinfo.signature = partition_info.Mbr.PartitionId;
            break;

        case PARTITION_STYLE_RAW:
            driveinfo.signature_type = DriveInfo::SIGNATURE::NONE;
            break;
        }

        if(!label.isEmpty())
            driveinfo.name += QString(" (%1)").arg(label);

        driveinfo.partition = partition_info.PartitionNumber;
        driveinfo.start = static_cast<uint64_t>(partition_info.StartingOffset.QuadPart);
        driveinfo.size = static_cast<uint64_t>(partition_info.PartitionLength.QuadPart);
        all.append(driveinfo);
    }

    FindVolumeClose(volume_handle);
    std::sort(std::begin(all), std::end(all));
    return all;
}
