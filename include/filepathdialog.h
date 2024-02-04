// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentry.h"
#include <QDialog>
#include <memory>

#include <QComboBox>
#include <QPlainTextEdit>
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
        PCCARD = 1,
        MEMORY_MAPPED = 2,
        CONTROLLER = 3,
        BMC = 4,
        ACPI = 5,
        EXPANDED = 6,
        ADR = 7,
        NVDIMM = 8,
        ATAPI = 9,
        SCSI = 10,
        FIBRE_CHANNEL = 11,
        FIREWIRE = 12,
        USB = 13,
        I2O = 14,
        INFINIBAND = 15,
        MAC_ADDRESS = 16,
        IPV4 = 17,
        IPV6 = 18,
        UART = 19,
        USB_CLASS = 20,
        USB_WWID = 21,
        DEVICE_LOGICAL_UNIT = 22,
        SATA = 23,
        ISCSI = 24,
        VLAN = 25,
        FIBRE_CHANNEL_EX = 26,
        SAS_EXTENDED_MESSAGING = 27,
        NVM_EXPRESS_NS = 28,
        URI = 29,
        UFS = 30,
        SD = 31,
        BLUETOOTH = 32,
        WI_FI = 33,
        EMMC = 34,
        BLUETOOTHLE = 35,
        DNS = 36,
        NVDIMM_NS = 37,
        REST_SERVICE = 38,
        NVME_OF_NS = 39,
        HD = 40,
        CD_ROM = 41,
        FILE_PATH = 42,
        PROTOCOL = 43,
        FIRMWARE_FILE = 44,
        FIRMWARE_VOLUME = 45,
        RELATIVE_OFFSET_RANGE = 46,
        RAM_DISK = 47,
        BOOT_SPECIFICATION = 48,
        VENDOR = 49,
        END = 50,
        UNKNOWN = 51,
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

    HorizontalTabStyle horizontal_tab_style{};
    std::unique_ptr<Ui::FilePathDialog> ui;
    int adr_additional_adr_format_index = 0;
    int dns_data_format_index = 0;
    int rest_service_data_format_index = 0;
    int vendor_data_format_index = 0;
    int unknown_data_format_index = 0;

public:
    explicit FilePathDialog(QWidget *parent = nullptr);
    FilePathDialog(const FilePathDialog &) = delete;
    FilePathDialog &operator=(const FilePathDialog &) = delete;
    ~FilePathDialog() override;

    void setReadOnly(bool readonly);

    FilePath::ANY toFilePath() const;

    void setFilePath(const FilePath::ANY *_file_path);
    void setPciForm(const FilePath::Pci &pci);
    void setPccardForm(const FilePath::Pccard &pccard);
    void setMemoryMappedForm(const FilePath::MemoryMapped &memory_mapped);
    void setControllerForm(const FilePath::Controller &controller);
    void setBmcForm(const FilePath::Bmc &bmc);
    void setAcpiForm(const FilePath::Acpi &acpi);
    void setExpandedForm(const FilePath::Expanded &expanded);
    void setAdrForm(const FilePath::Adr &adr);
    void setNvdimmForm(const FilePath::Nvdimm &nvdimm);
    void setAtapiForm(const FilePath::Atapi &atapi);
    void setScsiForm(const FilePath::Scsi &scsi);
    void setFibreChannelForm(const FilePath::FibreChannel &fibre_channel);
    void setFirewireForm(const FilePath::Firewire &firewire);
    void setUsbForm(const FilePath::Usb &usb);
    void setI2oForm(const FilePath::I2o &i2o);
    void setInfinibandForm(const FilePath::Infiniband &infiniband);
    void setMacAddressForm(const FilePath::MacAddress &mac_address);
    void setIpv4Form(const FilePath::Ipv4 &ipv4);
    void setIpv6Form(const FilePath::Ipv6 &ipv6);
    void setUartForm(const FilePath::Uart &uart);
    void setUsbClassForm(const FilePath::UsbClass &usb_class);
    void setUsbWwidForm(const FilePath::UsbWwid &usb_wwid);
    void setDeviceLogicalUnitForm(const FilePath::DeviceLogicalUnit &device_logical_unit);
    void setSataForm(const FilePath::Sata &sata);
    void setIscsiForm(const FilePath::Iscsi &iscsi);
    void setVlanForm(const FilePath::Vlan &vlan);
    void setFibreChannelExForm(const FilePath::FibreChannelEx &fibre_channel_ex);
    void setSasExtendedMessagingForm(const FilePath::SasExtendedMessaging &sas_extended_messaging);
    void setNvmExpressNsForm(const FilePath::NvmExpressNs &nvm_express_ns);
    void setUriForm(const FilePath::Uri &uri);
    void setUfsForm(const FilePath::Ufs &ufs);
    void setSdForm(const FilePath::Sd &sd);
    void setBluetoothForm(const FilePath::Bluetooth &bluetooth);
    void setWiFiForm(const FilePath::WiFi &wi_fi);
    void setEmmcForm(const FilePath::Emmc &emmc);
    void setBluetoothleForm(const FilePath::Bluetoothle &bluetoothle);
    void setDnsForm(const FilePath::Dns &dns);
    void setNvdimmNsForm(const FilePath::NvdimmNs &nvdimm_ns);
    void setRestServiceForm(const FilePath::RestService &rest_service);
    void setNvmeOfNsForm(const FilePath::NvmeOfNs &nvme_of_ns);
    void setHdForm(const FilePath::Hd &hd);
    void setCdRomForm(const FilePath::CdRom &cd_rom);
    void setFilePathForm(const FilePath::FilePath &file_path);
    void setProtocolForm(const FilePath::Protocol &protocol);
    void setFirmwareFileForm(const FilePath::FirmwareFile &firmware_file);
    void setFirmwareVolumeForm(const FilePath::FirmwareVolume &firmware_volume);
    void setRelativeOffsetRangeForm(const FilePath::RelativeOffsetRange &relative_offset_range);
    void setRamDiskForm(const FilePath::RamDisk &ram_disk);
    void setBootSpecificationForm(const FilePath::BootSpecification &boot_specification);
    void setVendorForm(const FilePath::Vendor &vendor);
    void setEndForm(const uint8_t subtype);
    void setUnknownForm(const FilePath::Unknown &unknown);

private:
    void resetForms();
    void resetPciForm();
    void resetPccardForm();
    void resetMemoryMappedForm();
    void resetControllerForm();
    void resetBmcForm();
    void resetAcpiForm();
    void resetExpandedForm();
    void resetAdrForm();
    void resetNvdimmForm();
    void resetAtapiForm();
    void resetScsiForm();
    void resetFibreChannelForm();
    void resetFirewireForm();
    void resetUsbForm();
    void resetI2oForm();
    void resetInfinibandForm();
    void resetMacAddressForm();
    void resetIpv4Form();
    void resetIpv6Form();
    void resetUartForm();
    void resetUsbClassForm();
    void resetUsbWwidForm();
    void resetDeviceLogicalUnitForm();
    void resetSataForm();
    void resetIscsiForm();
    void resetVlanForm();
    void resetFibreChannelExForm();
    void resetSasExtendedMessagingForm();
    void resetNvmExpressNsForm();
    void resetUriForm();
    void resetUfsForm();
    void resetSdForm();
    void resetBluetoothForm();
    void resetWiFiForm();
    void resetEmmcForm();
    void resetBluetoothleForm();
    void resetDnsForm();
    void resetNvdimmNsForm();
    void resetRestServiceForm();
    void resetNvmeOfNsForm();
    void resetHdForm();
    void resetCdRomForm();
    void resetFilePathForm();
    void resetProtocolForm();
    void resetFirmwareFileForm();
    void resetFirmwareVolumeForm();
    void resetRelativeOffsetRangeForm();
    void resetRamDiskForm();
    void resetBootSpecificationForm();
    void resetVendorForm();
    void resetEndForm();
    void resetUnknownForm();

    void refreshDiskCombo(bool force);
    QByteArray getData(const QPlainTextEdit &widget, int index) const;
    void dataFormatChanged(int &index, int new_index, QPlainTextEdit &data, QComboBox &format);

private slots:
    void resetDiskCombo();
    void diskChoiceChanged(int index);
    void signatureTypeChoiceChanged(int index);

    void AdrAdditionalAdrChanged(int index);
    void DnsDataChanged(int index);
    void RestServiceDataChanged(int index);
    void VendorDataFormatChanged(int index);
    void UnknownDataFormatChanged(int index);
};
