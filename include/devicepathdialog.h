// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentry.h"
#include <QDialog>
#include <memory>

namespace Ui
{
class DevicePathDialog;
}

class DevicePathDialog: public QDialog
{
    Q_OBJECT

private:
    std::unique_ptr<Ui::DevicePathDialog> ui;

public:
    explicit DevicePathDialog(QWidget *parent = nullptr);
    ~DevicePathDialog();

    Device_path::ANY toDevicePath() const;

    void setDevicePath(const Device_path::ANY *_device_path);
    void setPCIForm(const Device_path::PCI &pci);
    void setHIDForm(const Device_path::HID &hid);
    void setSATAForm(const Device_path::SATA &sata);
    void setHDForm(const Device_path::HD &hd);
    void setFileForm(const Device_path::File &file);
    void setFirmwareFileForm(const Device_path::FirmwareFile &firmware_file);
    void setFirmwareVolumeForm(const Device_path::FirmwareVolume &firmware_volume);

public slots:
    void resetDiskCombo();
    void diskChoiceChanged(int index);
    void signatureTypeChoiceChanged(int index);

private:
    void resetForms();
    void resetPCIForm();
    void resetHIDForm();
    void resetSATAForm();
    void resetHDForm();
    void resetFileForm();
    void resetFirmwareFileForm();
    void resetFirmwareVolumeForm();
    void refreshDiskCombo(bool force);
};
