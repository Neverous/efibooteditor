// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentry.h"
#include <QDialog>
#include <memory>

#include <QProxyStyle>
#include <QStyleOptionTab>

namespace Ui
{
class FilePathDialog;
}

class HorizontalTabStyle: public QProxyStyle
{
public:
    HorizontalTabStyle() = default;
    HorizontalTabStyle(const HorizontalTabStyle &) = delete;
    HorizontalTabStyle &operator=(const HorizontalTabStyle &) = delete;

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
};

class FilePathDialog: public QDialog
{
    Q_OBJECT

private:
    enum FormIndex
    {
        PCI = 0,
        HID = 1,
        USB = 2,
        Vendor = 3,
        MACAddress = 4,
        IPv4 = 5,
        IPv6 = 6,
        SATA = 7,
        HD = 8,
        File = 9,
        FirmwareFile = 10,
        FirmwareVolume = 11,
        BIOSBootSpecification = 12,
        End = 13,
        Unknown_ = 14,
    };

    enum DataFormat
    {
        Base64 = 0,
        Utf16 = 1,
        Utf8 = 2,
        Hex = 3,
    };

    enum class VendorTypeIndex : uint8_t
    {
        HW = 0,
        MSG = 1,
        MEDIA = 2,
    };

    using VendorDataFormat = DataFormat;
    using UnknownDataFormat = DataFormat;

    HorizontalTabStyle horizontal_tab_style;
    std::unique_ptr<Ui::FilePathDialog> ui;
    int vendor_data_format_combo_index;
    int unknown_data_format_combo_index;

public:
    explicit FilePathDialog(QWidget *parent = nullptr);
    FilePathDialog(const FilePathDialog &) = delete;
    FilePathDialog &operator=(const FilePathDialog &) = delete;
    ~FilePathDialog() override;

    void setReadOnly(bool readonly);

    File_path::ANY toFilePath() const;

    void setFilePath(const File_path::ANY *_file_path);
    void setPCIForm(const File_path::PCI &pci);
    void setHIDForm(const File_path::HID &hid);
    void setUSBForm(const File_path::USB &sub);
    void setVendorForm(const File_path::Vendor &vendor);
    void setMACAddressForm(const File_path::MACAddress &mac_address);
    void setIPv4Form(const File_path::IPv4 &ipv4);
    void setIPv6Form(const File_path::IPv6 &ipv6);
    void setSATAForm(const File_path::SATA &sata);
    void setHDForm(const File_path::HD &hd);
    void setFileForm(const File_path::File &file);
    void setFirmwareFileForm(const File_path::FirmwareFile &firmware_file);
    void setFirmwareVolumeForm(const File_path::FirmwareVolume &firmware_volume);
    void setBIOSBootSpecificationForm(const File_path::BIOSBootSpecification &bios_boot_specification);
    void setEndForm(const EFIBoot::EFIDP_END subtype);
    void setUnknownForm(const File_path::Unknown &unknown);

private:
    void resetForms();
    void resetPCIForm();
    void resetHIDForm();
    void resetUSBForm();
    QByteArray getVendorData(int index) const;
    QByteArray getUnknownData(int index) const;
    void resetVendorForm();
    void resetMACAddressForm();
    void resetIPv4Form();
    void resetIPv6Form();
    void resetSATAForm();
    void resetHDForm();
    void resetFileForm();
    void resetFirmwareFileForm();
    void resetFirmwareVolumeForm();
    void resetBIOSBootSpecification();
    void resetEndForm();
    void resetUnknownForm();
    void refreshDiskCombo(bool force);

private slots:
    void resetDiskCombo();
    void diskChoiceChanged(int index);
    void signatureTypeChoiceChanged(int index);
    void vendorDataFormatChanged(int index);
    void unknownDataFormatChanged(int index);
};
