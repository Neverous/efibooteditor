// SPDX-License-Identifier: LGPL-3.0-or-later
#include <Windows.h>
#include <array>

#include "compat.h"
#include "driveinfo.h"

QVector<DriveInfo> DriveInfo::getAll(bool refresh)
{
    if(!refresh && !all.empty())
        return all;

    all.clear();

    std::array<TCHAR, MAX_PATH> volume_name;
    HANDLE volume_handle = FindFirstVolume(volume_name.data(), static_cast<DWORD>(volume_name.size()));

    if(volume_handle == INVALID_HANDLE_VALUE)
        return all;

    for(BOOL volume_found = true; volume_found; volume_found = FindNextVolume(volume_handle, volume_name.data(), static_cast<DWORD>(volume_name.size())))
    {
        DriveInfo driveinfo{};
        std::array<TCHAR, MAX_PATH> device_name;
        size_t length = _tcsnccnt(volume_name.data(), volume_name.size());
        if(length != 49u)
            continue;

        volume_name[length - 1u] = _T('\0');
        if(!QueryDosDevice(&volume_name[4], device_name.data(), static_cast<DWORD>(device_name.size())))
            continue;

        volume_name[length - 1u] = _T('\\');

        PARTITION_INFORMATION_EX partition_information;

        device_name[4] = _T('\\');
        device_name[5] = _T('\\');
        device_name[6] = _T('.');
        HANDLE device_handle = CreateFile(&device_name[4], 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        device_name[4] = _T('i');
        device_name[5] = _T('c');
        device_name[6] = _T('e');
        if(device_handle == INVALID_HANDLE_VALUE)
            continue;

        if(!DeviceIoControl(device_handle, IOCTL_DISK_GET_PARTITION_INFO_EX, nullptr, 0, &partition_information, sizeof(partition_information), nullptr, nullptr))
        {
            CloseHandle(device_handle);
            continue;
        }

        CloseHandle(device_handle);
        driveinfo.name = QStringFromTCharArray(&device_name[8]);
        switch(partition_information.PartitionStyle)
        {
        case PARTITION_STYLE_GPT:
            driveinfo.signature_type = DriveInfo::SIGNATURE::GUID;
            driveinfo.name += QString(" (%1)").arg(QString::fromWCharArray(partition_information.Gpt.Name));
            driveinfo.signature = partition_information.Gpt.PartitionId;
            break;

        case PARTITION_STYLE_MBR:
            driveinfo.signature_type = DriveInfo::SIGNATURE::MBR;
            driveinfo.signature = partition_information.Mbr.PartitionId;
            break;

        case PARTITION_STYLE_RAW:
            driveinfo.signature_type = DriveInfo::SIGNATURE::NONE;
            break;
        }

        driveinfo.partition = partition_information.PartitionNumber;
        driveinfo.start = static_cast<uint64_t>(partition_information.StartingOffset.QuadPart);
        driveinfo.size = static_cast<uint64_t>(partition_information.PartitionLength.QuadPart);
        all.append(driveinfo);
    }

    FindVolumeClose(volume_handle);
    std::sort(std::begin(all), std::end(all));
    return all;
}
