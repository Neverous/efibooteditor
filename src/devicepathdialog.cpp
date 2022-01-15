// SPDX-License-Identifier: LGPL-3.0-or-later
#include "devicepathdialog.h"
#include "driveinfo.h"
#include "form/ui_devicepathdialog.h"

DevicePathDialog::DevicePathDialog(QWidget *parent)
    : QDialog(parent)
    , ui{std::make_unique<Ui::DevicePathDialog>()}
{
    ui->setupUi(this);
    setDevicePath(nullptr);
}

DevicePathDialog::~DevicePathDialog()
{
}

auto DevicePathDialog::toDevicePath() const -> Device_path::ANY
{
    switch(ui->options->currentIndex())
    {
    case 0:
    {
        Device_path::PCI pci;
        pci.function = static_cast<quint8>(ui->function_number->value());
        pci.device = static_cast<quint8>(ui->device_number->value());
        return pci;
    }
    case 1:
    {
        Device_path::HID hid;
        hid.hid = ui->hid_text->text().toUInt(nullptr, HEX_BASE);
        hid.uid = ui->uid_text->text().toUInt(nullptr, HEX_BASE);
        return hid;
    }
    case 2:
    {
        Device_path::SATA sata;
        sata.hba_port = static_cast<quint16>(ui->hba_port_number->value());
        sata.port_multiplier_port = static_cast<quint16>(ui->port_multiplier_port_number->value());
        sata.lun = static_cast<quint16>(ui->lun_number->value());
        return sata;
    }
    case 3:
    {
        Device_path::HD hd;
        hd.signature_type = static_cast<quint8>(ui->signature_type_combo->currentIndex());
        hd.partition_format = hd.signature_type ? hd.signature_type : static_cast<quint8>(EFIBoot::Device_path::SIGNATURE::MBR);
        hd.partition_number = static_cast<quint32>(ui->partition_number->value());
        switch(hd.signature_type)
        {
        case EFIBoot::Device_path::SIGNATURE::GUID:
            hd.partition_signature = QUuid::fromString(ui->signature_text->text());
            break;

        case EFIBoot::Device_path::SIGNATURE::MBR:
            hd.partition_signature = QUuid{ui->signature_text->text().toUInt(nullptr, HEX_BASE), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            break;
        }

        hd.partition_start = ui->start_text->text().toULongLong(nullptr, HEX_BASE);
        hd.partition_size = ui->size_text->text().toULongLong(nullptr, HEX_BASE);
        return hd;
    }

    case 4:
    {
        Device_path::File file;
        file.name = ui->filename_text->text();
        return file;
    }

    case 5:
    {
        Device_path::FirmwareFile firmware_file;
        firmware_file.name = QUuid::fromString(ui->firmware_file_text->text());
        return firmware_file;
    }

    case 6:
    {
        Device_path::FirmwareVolume firmware_volume;
        firmware_volume.name = QUuid::fromString(ui->firmware_volume_text->text());
        return firmware_volume;
    }
    }

    return {};
}

void DevicePathDialog::setDevicePath(const Device_path::ANY *_device_path)
{
    resetForms();
    if(!_device_path)
    {
        update();
        return;
    }

    std::visit(
        overloaded{
            // clang-format off
            [&](const Device_path::PCI &pci) { setPCIForm(pci); },
            [&](const Device_path::HID &hid) { setHIDForm(hid); },
            [&](const Device_path::SATA &sata) { setSATAForm(sata); },
            [&](const Device_path::HD &hd) { setHDForm(hd); },
            [&](const Device_path::File &file) { setFileForm(file); },
            [&](const Device_path::FirmwareFile &firmware_file) { setFirmwareFileForm(firmware_file); },
            [&](const Device_path::FirmwareVolume &firmware_volume) { setFirmwareVolumeForm(firmware_volume); },
            // clang-format on
        },
        *_device_path);

    update();
}

void DevicePathDialog::setPCIForm(const Device_path::PCI &pci)
{
    ui->options->setCurrentIndex(0);
    ui->function_number->setValue(pci.function);
    ui->device_number->setValue(pci.device);
}

void DevicePathDialog::setHIDForm(const Device_path::HID &hid)
{
    ui->options->setCurrentIndex(1);
    ui->hid_text->setText(QString::number(hid.hid, HEX_BASE));
    ui->uid_text->setText(QString::number(hid.uid, HEX_BASE));
}

void DevicePathDialog::setSATAForm(const Device_path::SATA &sata)
{
    ui->options->setCurrentIndex(2);
    ui->hba_port_number->setValue(sata.hba_port);
    ui->port_multiplier_port_number->setValue(sata.port_multiplier_port);
    ui->lun_number->setValue(sata.lun);
}

void DevicePathDialog::setHDForm(const Device_path::HD &hd)
{
    ui->options->setCurrentIndex(3);
    for(int index = 0; index < ui->disk_combo->count() - 2; ++index)
    {
        const auto &drive_info = ui->disk_combo->itemData(index).value<DriveInfo>();
        if(static_cast<quint8>(drive_info.signature_type) == hd.signature_type && drive_info.partition == hd.partition_number)
        {
            bool found = false;
            switch(drive_info.signature_type)
            {
            case DriveInfo::SIGNATURE::GUID:
                found = drive_info.signature == hd.partition_signature;
                break;
            case DriveInfo::SIGNATURE::MBR:
                found = drive_info.signature.data1 == hd.partition_signature.data1;
                break;
            case DriveInfo::SIGNATURE::NONE:
                break;
            }

            if(found)
            {
                ui->disk_combo->setCurrentIndex(index);
                emit diskChoiceChanged(index);
                return;
            }
        }
    }

    ui->disk_combo->setCurrentIndex(ui->disk_combo->count() - 1);
    emit diskChoiceChanged(ui->disk_combo->currentIndex());

    switch(hd.signature_type)
    {
    case EFIBoot::Device_path::SIGNATURE::NONE:
        ui->signature_type_combo->setCurrentIndex(0);
        break;

    case EFIBoot::Device_path::SIGNATURE::MBR:
        ui->signature_type_combo->setCurrentIndex(1);
        break;

    case EFIBoot::Device_path::SIGNATURE::GUID:
        ui->signature_type_combo->setCurrentIndex(2);
        break;
    }

    emit signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    if(hd.signature_type != EFIBoot::Device_path::SIGNATURE::NONE)
        ui->signature_text->setText(hd.partition_signature.toString());

    ui->partition_number->setValue(static_cast<int>(hd.partition_number));
    ui->start_text->setText(QString::number(hd.partition_start, HEX_BASE));
    ui->size_text->setText(QString::number(hd.partition_size, HEX_BASE));
}

void DevicePathDialog::setFileForm(const Device_path::File &file)
{
    ui->options->setCurrentIndex(4);
    ui->filename_text->setText(file.name);
}

void DevicePathDialog::setFirmwareFileForm(const Device_path::FirmwareFile &firmware_file)
{
    ui->options->setCurrentIndex(5);
    ui->firmware_file_text->setText(firmware_file.name.toString());
}

void DevicePathDialog::setFirmwareVolumeForm(const Device_path::FirmwareVolume &firmware_volume)
{
    ui->options->setCurrentIndex(6);
    ui->firmware_volume_text->setText(firmware_volume.name.toString());
}

void DevicePathDialog::refreshDiskCombo(bool force)
{
    ui->disk_combo->clear();
    for(const auto &drive: DriveInfo::get_all(force))
    {
        QVariant item;
        item.setValue(drive);
        ui->disk_combo->addItem(drive.name, item);
    }

    ui->disk_combo->insertSeparator(ui->disk_combo->count());
    ui->disk_combo->addItem("Custom");

    int index = ui->disk_combo->count() - 1;
    ui->disk_combo->setCurrentIndex(index);
    emit diskChoiceChanged(index);
}

void DevicePathDialog::resetDiskCombo()
{
    refreshDiskCombo(true);
}

void DevicePathDialog::diskChoiceChanged(int index)
{
    bool disabled = index + 1 != ui->disk_combo->count();
    ui->signature_type_combo->setDisabled(disabled);
    ui->signature_text->setDisabled(disabled);
    ui->partition_number->setDisabled(disabled);
    ui->start_text->setDisabled(disabled);
    ui->size_text->setDisabled(disabled);

    ui->signature_type_combo->setCurrentIndex(0);
    emit signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    ui->partition_number->clear();
    ui->start_text->clear();
    ui->size_text->clear();

    if(index + 1 == ui->disk_combo->count())
        return;

    const auto &driveinfo = ui->disk_combo->itemData(index).value<DriveInfo>();
    switch(driveinfo.signature_type)
    {
    case DriveInfo::SIGNATURE::NONE:
        ui->signature_type_combo->setCurrentIndex(0);
        break;

    case DriveInfo::SIGNATURE::MBR:
        ui->signature_type_combo->setCurrentIndex(1);
        break;

    case DriveInfo::SIGNATURE::GUID:
        ui->signature_type_combo->setCurrentIndex(2);
        break;
    }

    emit signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    if(driveinfo.signature_type != DriveInfo::SIGNATURE::NONE)
        ui->signature_text->setText(driveinfo.signature.toString());

    ui->partition_number->setValue(static_cast<int>(driveinfo.partition));
    ui->start_text->setText(QString::number(driveinfo.start, HEX_BASE));
    ui->size_text->setText(QString::number(driveinfo.size, HEX_BASE));
}

void DevicePathDialog::signatureTypeChoiceChanged(int index)
{
    switch(index)
    {
    case 0:
        ui->signature_text->setDisabled(true);
        ui->signature_text->setInputMask("");
        ui->signature_text->clear();
        break;

    case 1:
        ui->signature_text->setDisabled(ui->disk_combo->currentIndex() + 1 != ui->disk_combo->count());
        ui->signature_text->setInputMask("<\\0\\xHHHHHHHH;_");
        ui->signature_text->clear();
        break;

    case 2:
        ui->signature_text->setDisabled(ui->disk_combo->currentIndex() + 1 != ui->disk_combo->count());
        ui->signature_text->setInputMask("<HHHHHHHH-HHHH-HHHH-HHHH-HHHHHHHHHHHH;_");
        ui->signature_text->clear();
        break;
    }
}

void DevicePathDialog::resetForms()
{
    resetPCIForm();
    resetHIDForm();
    resetSATAForm();
    resetHDForm();
    resetFileForm();
    resetFirmwareFileForm();
    resetFirmwareVolumeForm();
}

void DevicePathDialog::resetPCIForm()
{
    ui->function_number->clear();
    ui->device_number->clear();
}

void DevicePathDialog::resetHIDForm()
{
    ui->hid_text->clear();
    ui->uid_text->clear();
}

void DevicePathDialog::resetSATAForm()
{
    ui->hba_port_number->clear();
    ui->port_multiplier_port_number->clear();
    ui->lun_number->clear();
}

void DevicePathDialog::resetHDForm()
{
    refreshDiskCombo(false);
    ui->signature_type_combo->setCurrentIndex(0);
    emit signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    ui->partition_number->clear();
    ui->start_text->clear();
    ui->size_text->clear();
}

void DevicePathDialog::resetFileForm()
{
    ui->filename_text->clear();
}

void DevicePathDialog::resetFirmwareFileForm()
{
    ui->firmware_file_text->clear();
}

void DevicePathDialog::resetFirmwareVolumeForm()
{
    ui->firmware_volume_text->setInputMask("<HHHHHHHH-HHHH-HHHH-HHHH-HHHHHHHHHHHH;_");
    ui->firmware_volume_text->clear();
}
