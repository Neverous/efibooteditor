// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentry.h"
#include <QDialog>
#include <memory>

#include <QProxyStyle>
#include <QStyleOptionTab>

namespace Ui
{
class DevicePathDialog;
}

class HorizontalTabStyle: public QProxyStyle
{
public:
    HorizontalTabStyle() = default;
    HorizontalTabStyle(const HorizontalTabStyle &) = delete;
    HorizontalTabStyle &operator=(const HorizontalTabStyle &) = delete;

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if(type == QStyle::CT_TabBarTab)
            s.transpose();

        return s;
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override
    {
        if(element != CE_TabBarTabLabel)
        {
            QProxyStyle::drawControl(element, option, painter, widget);
            return;
        }

        const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option);
        if(!tab)
        {
            QProxyStyle::drawControl(element, option, painter, widget);
            return;
        }

        QStyleOptionTab opt(*tab);
        opt.shape = QTabBar::RoundedNorth;
        QProxyStyle::drawControl(element, &opt, painter, widget);
    }
};

#if defined(_MSC_VER)
#pragma warning(push)
// C4820: 'bytes' bytes padding added after construct 'member_name'
#pragma warning(disable : 4820)
#endif

class DevicePathDialog: public QDialog
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

    enum VendorTypeIndex
    {
        HW = 0,
        MSG = 1,
        MEDIA = 2,
    };

    typedef DataFormat VendorDataFormat;
    typedef DataFormat UnknownDataFormat;

    HorizontalTabStyle horizontal_tab_style;
    std::unique_ptr<Ui::DevicePathDialog> ui;
    int vendor_data_format_combo_index;
    int unknown_data_format_combo_index;

public:
    explicit DevicePathDialog(QWidget *parent = nullptr);
    DevicePathDialog(const DevicePathDialog &) = delete;
    DevicePathDialog &operator=(const DevicePathDialog &) = delete;
    ~DevicePathDialog() override;

    Device_path::ANY toDevicePath() const;

    void setDevicePath(const Device_path::ANY *_device_path);
    void setPCIForm(const Device_path::PCI &pci);
    void setHIDForm(const Device_path::HID &hid);
    void setUSBForm(const Device_path::USB &sub);
    void setVendorForm(const Device_path::Vendor &vendor);
    void setMACAddressForm(const Device_path::MACAddress &mac_address);
    void setIPv4Form(const Device_path::IPv4 &ipv4);
    void setIPv6Form(const Device_path::IPv6 &ipv6);
    void setSATAForm(const Device_path::SATA &sata);
    void setHDForm(const Device_path::HD &hd);
    void setFileForm(const Device_path::File &file);
    void setFirmwareFileForm(const Device_path::FirmwareFile &firmware_file);
    void setFirmwareVolumeForm(const Device_path::FirmwareVolume &firmware_volume);
    void setBIOSBootSpecificationForm(const Device_path::BIOSBootSpecification &bios_boot_specification);
    void setEndForm(const EFIBoot::EFIDP_END subtype);
    void setUnknownForm(const Device_path::Unknown &unknown);

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

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
