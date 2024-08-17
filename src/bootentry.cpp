// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "bootentry.h"

#include <QJsonArray>
#include <QJsonObject>

#include "efiboot.h"

EFIBoot::Progress_fn EFIBoot::_get_variables_progress_fn = nullptr;

#define check_obj()                                      \
    if(obj["type"] != TYPE || obj["subtype"] != SUBTYPE) \
    return std::nullopt
#define check_type(field, typecheck)                          \
    if(!obj.contains(#field) || !obj[#field].is##typecheck()) \
    return std::nullopt
#define try_read(field, typecheck) try_read_3(field, typecheck, typecheck)
#define try_read_3(field, typecheck, typecast) \
    check_type(field, typecheck);              \
    value.field = static_cast<decltype(value.field)>(obj[#field].to##typecast())
#define try_read_4(field, old_name, typecheck, typecast)                                \
    if(obj.contains(old_name) && obj[old_name].is##typecheck())                         \
        value.field = static_cast<decltype(value.field)>(obj[old_name].to##typecast()); \
    else if(obj.contains(#field) && obj[#field].is##typecheck())                        \
        value.field = static_cast<decltype(value.field)>(obj[#field].to##typecast());   \
    else                                                                                \
        return std::nullopt

// Hardware

FilePath::Pci::Pci(const EFIBoot::File_path::HW::Pci &_pci)
    : _string{}
    , function{_pci.function}
    , device{_pci.device}
{
}

auto FilePath::Pci::toEFIBootFilePath() const -> EFIBoot::File_path::HW::Pci
{
    EFIBoot::File_path::HW::Pci value{};
    value.function = function;
    value.device = device;
    return value;
}

auto FilePath::Pci::fromJSON(const QJsonObject &obj) -> std::optional<Pci>
{
    Pci value{};
    check_obj();
    try_read_3(function, Double, Int);
    try_read_3(device, Double, Int);
    return {value};
}

auto FilePath::Pci::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["function"] = static_cast<int>(function);
    value["device"] = static_cast<int>(device);
    return value;
}

auto FilePath::Pci::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Pci(%1,%2)").arg(device).arg(function);
}

FilePath::Pccard::Pccard(const EFIBoot::File_path::HW::Pccard &_pccard)
    : _string{}
    , function_number{_pccard.function_number}
{
}

auto FilePath::Pccard::toEFIBootFilePath() const -> EFIBoot::File_path::HW::Pccard
{
    EFIBoot::File_path::HW::Pccard value{};
    value.function_number = function_number;
    return value;
}

auto FilePath::Pccard::fromJSON(const QJsonObject &obj) -> std::optional<Pccard>
{
    Pccard value{};
    check_obj();
    try_read_3(function_number, Double, Int);
    return {value};
}

auto FilePath::Pccard::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["function_number"] = static_cast<int>(function_number);
    return value;
}

auto FilePath::Pccard::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("PcCard(%1)").arg(function_number);
}

FilePath::MemoryMapped::MemoryMapped(const EFIBoot::File_path::HW::Memory_mapped &_memory_mapped)
    : _string{}
    , memory_type{_memory_mapped.memory_type}
    , start_address{_memory_mapped.start_address}
    , end_address{_memory_mapped.end_address}
{
}

auto FilePath::MemoryMapped::toEFIBootFilePath() const -> EFIBoot::File_path::HW::Memory_mapped
{
    EFIBoot::File_path::HW::Memory_mapped value{};
    value.memory_type = memory_type;
    value.start_address = start_address;
    value.end_address = end_address;
    return value;
}

auto FilePath::MemoryMapped::fromJSON(const QJsonObject &obj) -> std::optional<MemoryMapped>
{
    MemoryMapped value{};
    check_obj();
    try_read_3(memory_type, Double, Int);
    check_type(start_address, String);
    value.start_address = obj["start_address"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(end_address, String);
    value.end_address = obj["end_address"].toString().toULongLong(nullptr, HEX_BASE);
    return {value};
}

auto FilePath::MemoryMapped::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["memory_type"] = static_cast<int>(memory_type);
    value["start_address"] = toHex(start_address);
    value["end_address"] = toHex(end_address);
    return value;
}

auto FilePath::MemoryMapped::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("MemoryMapped(%1,%2,%3)").arg(static_cast<int>(memory_type)).arg(start_address).arg(end_address);
}

FilePath::Controller::Controller(const EFIBoot::File_path::HW::Controller &_controller)
    : _string{}
    , controller_number{_controller.controller_number}
{
}

auto FilePath::Controller::toEFIBootFilePath() const -> EFIBoot::File_path::HW::Controller
{
    EFIBoot::File_path::HW::Controller value{};
    value.controller_number = controller_number;
    return value;
}

auto FilePath::Controller::fromJSON(const QJsonObject &obj) -> std::optional<Controller>
{
    Controller value{};
    check_obj();
    try_read_3(controller_number, Double, Int);
    return {value};
}

auto FilePath::Controller::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["controller_number"] = static_cast<int>(controller_number);
    return value;
}

auto FilePath::Controller::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Ctrl(%1)").arg(controller_number);
}

FilePath::Bmc::Bmc(const EFIBoot::File_path::HW::Bmc &_bmc)
    : _string{}
    , interface_type{_bmc.interface_type}
    , base_address{_bmc.base_address}
{
}

auto FilePath::Bmc::toEFIBootFilePath() const -> EFIBoot::File_path::HW::Bmc
{
    EFIBoot::File_path::HW::Bmc value{};
    value.interface_type = interface_type;
    value.base_address = base_address;
    return value;
}

auto FilePath::Bmc::fromJSON(const QJsonObject &obj) -> std::optional<Bmc>
{
    Bmc value{};
    check_obj();
    try_read_3(interface_type, Double, Int);
    check_type(base_address, String);
    value.base_address = obj["base_address"].toString().toULongLong(nullptr, HEX_BASE);
    return {value};
}

auto FilePath::Bmc::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["interface_type"] = static_cast<int>(interface_type);
    value["base_address"] = toHex(base_address);
    return value;
}

auto FilePath::Bmc::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("BMC(%1,%2)").arg(static_cast<int>(interface_type)).arg(base_address);
}

// ACPI

FilePath::Acpi::Acpi(const EFIBoot::File_path::ACPI::Acpi &_acpi)
    : _string{}
    , hid{_acpi.hid}
    , uid{_acpi.uid}
{
}

auto FilePath::Acpi::toEFIBootFilePath() const -> EFIBoot::File_path::ACPI::Acpi
{
    EFIBoot::File_path::ACPI::Acpi value{};
    value.hid = hid;
    value.uid = uid;
    return value;
}

auto FilePath::Acpi::fromJSON(const QJsonObject &obj) -> std::optional<Acpi>
{
    Acpi value{};
    if(obj["type"] != TYPE)
        return std::nullopt;

    // Support for old names
    if(obj["subtype"] != SUBTYPE && obj["subtype"] != "HID")
        return std::nullopt;

    // check_obj();
    try_read_3(hid, Double, Int);
    try_read_3(uid, Double, Int);
    return {value};
}

auto FilePath::Acpi::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["hid"] = static_cast<int>(hid);
    value["uid"] = static_cast<int>(uid);
    return value;
}

auto FilePath::Acpi::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Acpi(%1,%2)").arg(toHex(hid), toHex(uid));
}

FilePath::Expanded::Expanded(const EFIBoot::File_path::ACPI::Expanded &_expanded)
    : _string{}
    , hid{_expanded.hid}
    , uid{_expanded.uid}
    , cid{_expanded.cid}
    , hidstr{QByteArray::fromRawData(_expanded.hidstr.data(), static_cast<int>(_expanded.hidstr.size() * sizeof(decltype(_expanded.hidstr)::value_type)))}
    , uidstr{QByteArray::fromRawData(_expanded.uidstr.data(), static_cast<int>(_expanded.uidstr.size() * sizeof(decltype(_expanded.uidstr)::value_type)))}
    , cidstr{QByteArray::fromRawData(_expanded.cidstr.data(), static_cast<int>(_expanded.cidstr.size() * sizeof(decltype(_expanded.cidstr)::value_type)))}
{
}

auto FilePath::Expanded::toEFIBootFilePath() const -> EFIBoot::File_path::ACPI::Expanded
{
    EFIBoot::File_path::ACPI::Expanded value{};
    value.hid = hid;
    value.uid = uid;
    value.cid = cid;
    value.hidstr = hidstr.toStdString();
    value.uidstr = uidstr.toStdString();
    value.cidstr = cidstr.toStdString();
    return value;
}

auto FilePath::Expanded::fromJSON(const QJsonObject &obj) -> std::optional<Expanded>
{
    Expanded value{};
    check_obj();
    try_read_3(hid, Double, Int);
    try_read_3(uid, Double, Int);
    try_read_3(cid, Double, Int);
    try_read(hidstr, String);
    try_read(uidstr, String);
    try_read(cidstr, String);
    return {value};
}

auto FilePath::Expanded::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["hid"] = static_cast<int>(hid);
    value["uid"] = static_cast<int>(uid);
    value["cid"] = static_cast<int>(cid);
    value["hidstr"] = hidstr;
    value["uidstr"] = uidstr;
    value["cidstr"] = cidstr;
    return value;
}

auto FilePath::Expanded::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("AcpiEx(%1,%2,%3,%4,%5,%6)").arg(hid).arg(cid).arg(uid).arg(hidstr).arg(cidstr).arg(uidstr);
}

FilePath::Adr::Adr(const EFIBoot::File_path::ACPI::Adr &_adr)
    : _string{}
    , adr{_adr.adr}
    , additional_adr{QByteArray::fromRawData(reinterpret_cast<const char *>(_adr.additional_adr.data()), static_cast<int>(_adr.additional_adr.size() * sizeof(decltype(_adr.additional_adr)::value_type)))}
{
    additional_adr.detach();
}

auto FilePath::Adr::toEFIBootFilePath() const -> EFIBoot::File_path::ACPI::Adr
{
    EFIBoot::File_path::ACPI::Adr value{};
    value.adr = adr;
    value.additional_adr = {additional_adr.begin(), additional_adr.end()};
    return value;
}

auto FilePath::Adr::fromJSON(const QJsonObject &obj) -> std::optional<Adr>
{
    Adr value{};
    check_obj();
    try_read_3(adr, Double, Int);
    check_type(additional_adr, String);
    value.additional_adr = QByteArray::fromBase64(obj["additional_adr"].toString().toUtf8());
    return {value};
}

auto FilePath::Adr::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["adr"] = static_cast<int>(adr);
    value["additional_adr"] = static_cast<QString>(additional_adr.toBase64());
    return value;
}

auto FilePath::Adr::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("AcpiAdr(%1,[%2B])").arg(adr).arg(additional_adr.size());
}

FilePath::Nvdimm::Nvdimm(const EFIBoot::File_path::ACPI::Nvdimm &_nvdimm)
    : _string{}
    , nfit_device_handle{_nvdimm.nfit_device_handle}
{
}

auto FilePath::Nvdimm::toEFIBootFilePath() const -> EFIBoot::File_path::ACPI::Nvdimm
{
    EFIBoot::File_path::ACPI::Nvdimm value{};
    value.nfit_device_handle = nfit_device_handle;
    return value;
}

auto FilePath::Nvdimm::fromJSON(const QJsonObject &obj) -> std::optional<Nvdimm>
{
    Nvdimm value{};
    check_obj();
    try_read_3(nfit_device_handle, Double, Int);
    return {value};
}

auto FilePath::Nvdimm::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["nfit_device_handle"] = static_cast<int>(nfit_device_handle);
    return value;
}

auto FilePath::Nvdimm::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Nvdimm(%1)").arg(nfit_device_handle);
}

// Messaging

FilePath::Atapi::Atapi(const EFIBoot::File_path::MSG::Atapi &_atapi)
    : _string{}
    , primary{_atapi.primary}
    , slave{_atapi.slave}
    , lun{_atapi.lun}
{
}

auto FilePath::Atapi::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Atapi
{
    EFIBoot::File_path::MSG::Atapi value{};
    value.primary = primary;
    value.slave = slave;
    value.lun = lun;
    return value;
}

auto FilePath::Atapi::fromJSON(const QJsonObject &obj) -> std::optional<Atapi>
{
    Atapi value{};
    check_obj();
    try_read(primary, Bool);
    try_read(slave, Bool);
    try_read_3(lun, Double, Int);
    return {value};
}

auto FilePath::Atapi::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["primary"] = primary;
    value["slave"] = slave;
    value["lun"] = static_cast<int>(lun);
    return value;
}

auto FilePath::Atapi::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Ata(%1,%2,%3)").arg(primary).arg(slave).arg(lun);
}

FilePath::Scsi::Scsi(const EFIBoot::File_path::MSG::Scsi &_scsi)
    : _string{}
    , pun{_scsi.pun}
    , lun{_scsi.lun}
{
}

auto FilePath::Scsi::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Scsi
{
    EFIBoot::File_path::MSG::Scsi value{};
    value.pun = pun;
    value.lun = lun;
    return value;
}

auto FilePath::Scsi::fromJSON(const QJsonObject &obj) -> std::optional<Scsi>
{
    Scsi value{};
    check_obj();
    try_read_3(pun, Double, Int);
    try_read_3(lun, Double, Int);
    return {value};
}

auto FilePath::Scsi::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["pun"] = static_cast<int>(pun);
    value["lun"] = static_cast<int>(lun);
    return value;
}

auto FilePath::Scsi::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Scsi(%1,%2)").arg(pun).arg(lun);
}

FilePath::FibreChannel::FibreChannel(const EFIBoot::File_path::MSG::Fibre_channel &_fibre_channel)
    : _string{}
    , reserved{_fibre_channel.reserved}
    , world_wide_name{_fibre_channel.world_wide_name}
    , lun{_fibre_channel.lun}
{
}

auto FilePath::FibreChannel::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Fibre_channel
{
    EFIBoot::File_path::MSG::Fibre_channel value{};
    value.reserved = reserved;
    value.world_wide_name = world_wide_name;
    value.lun = lun;
    return value;
}

auto FilePath::FibreChannel::fromJSON(const QJsonObject &obj) -> std::optional<FibreChannel>
{
    FibreChannel value{};
    check_obj();
    try_read_3(reserved, Double, Int);
    check_type(world_wide_name, String);
    value.world_wide_name = obj["world_wide_name"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(lun, String);
    value.lun = obj["lun"].toString().toULongLong(nullptr, HEX_BASE);
    return {value};
}

auto FilePath::FibreChannel::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["reserved"] = static_cast<int>(reserved);
    value["world_wide_name"] = toHex(world_wide_name);
    value["lun"] = toHex(lun);
    return value;
}

auto FilePath::FibreChannel::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Fibre(%1,%2)").arg(world_wide_name).arg(lun);
}

FilePath::Firewire::Firewire(const EFIBoot::File_path::MSG::Firewire &_firewire)
    : _string{}
    , reserved{_firewire.reserved}
    , guid{_firewire.guid}
{
}

auto FilePath::Firewire::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Firewire
{
    EFIBoot::File_path::MSG::Firewire value{};
    value.reserved = reserved;
    value.guid = guid;
    return value;
}

auto FilePath::Firewire::fromJSON(const QJsonObject &obj) -> std::optional<Firewire>
{
    Firewire value{};
    check_obj();
    try_read_3(reserved, Double, Int);
    check_type(guid, String);
    value.guid = obj["guid"].toString().toULongLong(nullptr, HEX_BASE);
    return {value};
}

auto FilePath::Firewire::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["reserved"] = static_cast<int>(reserved);
    value["guid"] = toHex(guid);
    return value;
}

auto FilePath::Firewire::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("I1394(%1)").arg(guid);
}

FilePath::Usb::Usb(const EFIBoot::File_path::MSG::Usb &_usb)
    : _string{}
    , parent_port_number{_usb.parent_port_number}
    , interface_number{_usb.interface_number}
{
}

auto FilePath::Usb::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Usb
{
    EFIBoot::File_path::MSG::Usb value{};
    value.parent_port_number = parent_port_number;
    value.interface_number = interface_number;
    return value;
}

auto FilePath::Usb::fromJSON(const QJsonObject &obj) -> std::optional<Usb>
{
    Usb value{};
    check_obj();
    try_read_3(parent_port_number, Double, Int);
    // Support for old names
    try_read_4(interface_number, "interface", Double, Int);
    return {value};
}

auto FilePath::Usb::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["parent_port_number"] = static_cast<int>(parent_port_number);
    value["interface_number"] = static_cast<int>(interface_number);
    return value;
}

auto FilePath::Usb::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("USB(%1,%2)").arg(parent_port_number).arg(interface_number);
}

FilePath::I2o::I2o(const EFIBoot::File_path::MSG::I2o &_i2o)
    : _string{}
    , tid{_i2o.tid}
{
}

auto FilePath::I2o::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::I2o
{
    EFIBoot::File_path::MSG::I2o value{};
    value.tid = tid;
    return value;
}

auto FilePath::I2o::fromJSON(const QJsonObject &obj) -> std::optional<I2o>
{
    I2o value{};
    check_obj();
    try_read_3(tid, Double, Int);
    return {value};
}

auto FilePath::I2o::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["tid"] = static_cast<int>(tid);
    return value;
}

auto FilePath::I2o::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("I2O(%1)").arg(tid);
}

FilePath::Infiniband::Infiniband(const EFIBoot::File_path::MSG::Infiniband &_infiniband)
    : _string{}
    , resource_flags{_infiniband.resource_flags}
    , port_gid{}
    , ioc_guid_service_id{_infiniband.ioc_guid_service_id}
    , target_port_id{_infiniband.target_port_id}
    , device_id{_infiniband.device_id}
{
    static_assert(sizeof(port_gid) == sizeof(_infiniband.port_gid));
    memcpy(reinterpret_cast<void *>(&port_gid), &_infiniband.port_gid, sizeof(port_gid));
}

auto FilePath::Infiniband::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Infiniband
{
    EFIBoot::File_path::MSG::Infiniband value{};
    value.resource_flags = resource_flags;
    static_assert(sizeof(port_gid) == sizeof(value.port_gid));
    memcpy(value.port_gid.data(), &port_gid, sizeof(value.port_gid));
    value.ioc_guid_service_id = ioc_guid_service_id;
    value.target_port_id = target_port_id;
    value.device_id = device_id;
    return value;
}

auto FilePath::Infiniband::fromJSON(const QJsonObject &obj) -> std::optional<Infiniband>
{
    Infiniband value{};
    check_obj();
    try_read_3(resource_flags, Double, Int);
    check_type(port_gid, String);
    value.port_gid = QUuid::fromString(obj["port_gid"].toString());
    check_type(ioc_guid_service_id, String);
    value.ioc_guid_service_id = obj["ioc_guid_service_id"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(target_port_id, String);
    value.target_port_id = obj["target_port_id"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(device_id, String);
    value.device_id = obj["device_id"].toString().toULongLong(nullptr, HEX_BASE);
    return {value};
}

auto FilePath::Infiniband::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["resource_flags"] = static_cast<int>(resource_flags);
    value["port_gid"] = port_gid.toString();
    value["ioc_guid_service_id"] = toHex(ioc_guid_service_id);
    value["target_port_id"] = toHex(target_port_id);
    value["device_id"] = toHex(device_id);
    return value;
}

auto FilePath::Infiniband::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Infiniband(%1,%2,%3,%4,%5)").arg(resource_flags).arg(port_gid.toString(QUuid::WithoutBraces)).arg(ioc_guid_service_id).arg(target_port_id).arg(device_id);
}

FilePath::MacAddress::MacAddress(const EFIBoot::File_path::MSG::Mac_address &_mac_address)
    : _string{}
    , address{QByteArray::fromRawData(reinterpret_cast<const char *>(_mac_address.address.data()), static_cast<int>(_mac_address.address.size() * sizeof(decltype(_mac_address.address)::value_type)))}
    , if_type{_mac_address.if_type}
{
}

auto FilePath::MacAddress::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Mac_address
{
    EFIBoot::File_path::MSG::Mac_address value{};
    {
        auto bytes = QByteArray::fromHex(address.toUtf8());
        memcpy(value.address.data(), bytes.data(), qMin(static_cast<size_t>(bytes.size()), sizeof(value.address)));
    }
    value.if_type = if_type;
    return value;
}

auto FilePath::MacAddress::fromJSON(const QJsonObject &obj) -> std::optional<MacAddress>
{
    MacAddress value{};
    check_obj();
    try_read(address, String);
    try_read_3(if_type, Double, Int);
    return {value};
}

auto FilePath::MacAddress::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["address"] = address;
    value["if_type"] = static_cast<int>(if_type);
    return value;
}

auto FilePath::MacAddress::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("MAC(%1,%2)").arg(address.left(12)).arg(if_type);
}

FilePath::Ipv4::Ipv4(const EFIBoot::File_path::MSG::Ipv4 &_ipv4)
    : _string{}
    , local_ip_address{*reinterpret_cast<const uint32_t *>(_ipv4.local_ip_address.data())}
    , remote_ip_address{*reinterpret_cast<const uint32_t *>(_ipv4.remote_ip_address.data())}
    , local_port{_ipv4.local_port}
    , remote_port{_ipv4.remote_port}
    , protocol{_ipv4.protocol}
    , static_ip_address{_ipv4.static_ip_address}
    , gateway_ip_address{*reinterpret_cast<const uint32_t *>(_ipv4.gateway_ip_address.data())}
    , subnet_mask{*reinterpret_cast<const uint32_t *>(_ipv4.subnet_mask.data())}
{
    static_assert(sizeof(local_ip_address.toIPv4Address()) == sizeof(_ipv4.local_ip_address));
    static_assert(sizeof(remote_ip_address.toIPv4Address()) == sizeof(_ipv4.remote_ip_address));
    static_assert(sizeof(gateway_ip_address.toIPv4Address()) == sizeof(_ipv4.gateway_ip_address));
    static_assert(sizeof(subnet_mask.toIPv4Address()) == sizeof(_ipv4.subnet_mask));
}

auto FilePath::Ipv4::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Ipv4
{
    EFIBoot::File_path::MSG::Ipv4 value{};
    {
        auto ip_address = local_ip_address.toIPv4Address();
        static_assert(sizeof(ip_address) == sizeof(value.local_ip_address));
        memcpy(value.local_ip_address.data(), &ip_address, sizeof(value.local_ip_address));
    }
    {
        auto ip_address = remote_ip_address.toIPv4Address();
        static_assert(sizeof(ip_address) == sizeof(value.remote_ip_address));
        memcpy(value.remote_ip_address.data(), &ip_address, sizeof(value.remote_ip_address));
    }
    value.local_port = local_port;
    value.remote_port = remote_port;
    value.protocol = protocol;
    value.static_ip_address = static_ip_address;
    {
        auto ip_address = gateway_ip_address.toIPv4Address();
        static_assert(sizeof(ip_address) == sizeof(value.gateway_ip_address));
        memcpy(value.gateway_ip_address.data(), &ip_address, sizeof(value.gateway_ip_address));
    }
    {
        auto ip_address = subnet_mask.toIPv4Address();
        static_assert(sizeof(ip_address) == sizeof(value.subnet_mask));
        memcpy(value.subnet_mask.data(), &ip_address, sizeof(value.subnet_mask));
    }
    return value;
}

auto FilePath::Ipv4::fromJSON(const QJsonObject &obj) -> std::optional<Ipv4>
{
    Ipv4 value{};
    check_obj();
    try_read(local_ip_address, String);
    try_read(remote_ip_address, String);
    try_read_3(local_port, Double, Int);
    try_read_3(remote_port, Double, Int);
    try_read_3(protocol, Double, Int);
    try_read(static_ip_address, Bool);
    try_read(gateway_ip_address, String);
    try_read(subnet_mask, String);
    return {value};
}

auto FilePath::Ipv4::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["local_ip_address"] = local_ip_address.toString();
    value["remote_ip_address"] = remote_ip_address.toString();
    value["local_port"] = static_cast<int>(local_port);
    value["remote_port"] = static_cast<int>(remote_port);
    value["protocol"] = static_cast<int>(protocol);
    value["static_ip_address"] = static_ip_address;
    value["gateway_ip_address"] = gateway_ip_address.toString();
    value["subnet_mask"] = subnet_mask.toString();
    return value;
}

auto FilePath::Ipv4::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("IPv4(%1:%2,%3,%4,%5:%6,%7,%8)")
                         .arg(remote_ip_address.toString())
                         .arg(remote_port)
                         .arg(protocol == 17 ? "UDP" : (protocol == 6 ? "TCP" : QString::number(protocol)))
                         .arg(static_ip_address ? "Static" : "DHCP")
                         .arg(local_ip_address.toString())
                         .arg(local_port)
                         .arg(gateway_ip_address.toString())
                         .arg(subnet_mask.toString());
}

FilePath::Ipv6::Ipv6(const EFIBoot::File_path::MSG::Ipv6 &_ipv6)
    : _string{}
    , local_ip_address{_ipv6.local_ip_address.data()}
    , remote_ip_address{_ipv6.remote_ip_address.data()}
    , local_port{_ipv6.local_port}
    , remote_port{_ipv6.remote_port}
    , protocol{_ipv6.protocol}
    , ip_address_origin{_ipv6.ip_address_origin}
    , prefix_length{_ipv6.prefix_length}
    , gateway_ip_address{_ipv6.gateway_ip_address.data()}
{
    static_assert(sizeof(local_ip_address.toIPv6Address()) == sizeof(_ipv6.local_ip_address));
    static_assert(sizeof(remote_ip_address.toIPv6Address()) == sizeof(_ipv6.remote_ip_address));
    static_assert(sizeof(gateway_ip_address.toIPv6Address()) == sizeof(_ipv6.gateway_ip_address));
}

auto FilePath::Ipv6::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Ipv6
{
    EFIBoot::File_path::MSG::Ipv6 value{};
    {
        auto ip_address = local_ip_address.toIPv6Address();
        static_assert(sizeof(ip_address) == sizeof(value.local_ip_address));
        memcpy(value.local_ip_address.data(), &ip_address, sizeof(value.local_ip_address));
    }
    {
        auto ip_address = remote_ip_address.toIPv6Address();
        static_assert(sizeof(ip_address) == sizeof(value.remote_ip_address));
        memcpy(value.remote_ip_address.data(), &ip_address, sizeof(value.remote_ip_address));
    }
    value.local_port = local_port;
    value.remote_port = remote_port;
    value.protocol = protocol;
    value.ip_address_origin = ip_address_origin;
    value.prefix_length = prefix_length;
    {
        auto ip_address = gateway_ip_address.toIPv6Address();
        static_assert(sizeof(ip_address) == sizeof(value.gateway_ip_address));
        memcpy(value.gateway_ip_address.data(), &ip_address, sizeof(value.gateway_ip_address));
    }
    return value;
}

auto FilePath::Ipv6::fromJSON(const QJsonObject &obj) -> std::optional<Ipv6>
{
    Ipv6 value{};
    check_obj();
    try_read(local_ip_address, String);
    try_read(remote_ip_address, String);
    try_read_3(local_port, Double, Int);
    try_read_3(remote_port, Double, Int);
    try_read_3(protocol, Double, Int);
    try_read_3(ip_address_origin, Double, Int);
    try_read_3(prefix_length, Double, Int);
    try_read(gateway_ip_address, String);
    return {value};
}

auto FilePath::Ipv6::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["local_ip_address"] = local_ip_address.toString();
    value["remote_ip_address"] = remote_ip_address.toString();
    value["local_port"] = static_cast<int>(local_port);
    value["remote_port"] = static_cast<int>(remote_port);
    value["protocol"] = static_cast<int>(protocol);
    value["ip_address_origin"] = static_cast<int>(ip_address_origin);
    value["prefix_length"] = static_cast<int>(prefix_length);
    value["gateway_ip_address"] = gateway_ip_address.toString();
    return value;
}

auto FilePath::Ipv6::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("IPv6(%1:%2,%3,%4,%5:%6,%7,%8)")
                         .arg(remote_ip_address.toString())
                         .arg(remote_port)
                         .arg(protocol == 17 ? "UDP" : (protocol == 6 ? "TCP" : QString::number(protocol)))
                         .arg(ip_address_origin == EFIBoot::File_path::MSG::Ipv6::IP_ADDRESS_ORIGIN::STATIC ? "Static" : (ip_address_origin == EFIBoot::File_path::MSG::Ipv6::IP_ADDRESS_ORIGIN::STATELESS ? "StatelessAutoConfigure" : (ip_address_origin == EFIBoot::File_path::MSG::Ipv6::IP_ADDRESS_ORIGIN::STATEFUL ? "StatefulAutoConfigure" : QString::number(static_cast<int>(ip_address_origin)))))
                         .arg(local_ip_address.toString())
                         .arg(local_port)
                         .arg(gateway_ip_address.toString())
                         .arg(prefix_length);
}

FilePath::Uart::Uart(const EFIBoot::File_path::MSG::Uart &_uart)
    : _string{}
    , reserved{_uart.reserved}
    , baud_rate{_uart.baud_rate}
    , data_bits{_uart.data_bits}
    , parity{_uart.parity}
    , stop_bits{_uart.stop_bits}
{
}

auto FilePath::Uart::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Uart
{
    EFIBoot::File_path::MSG::Uart value{};
    value.reserved = reserved;
    value.baud_rate = baud_rate;
    value.data_bits = data_bits;
    value.parity = parity;
    value.stop_bits = stop_bits;
    return value;
}

auto FilePath::Uart::fromJSON(const QJsonObject &obj) -> std::optional<Uart>
{
    Uart value{};
    check_obj();
    try_read_3(reserved, Double, Int);
    check_type(baud_rate, String);
    value.baud_rate = obj["baud_rate"].toString().toULongLong(nullptr, HEX_BASE);
    try_read_3(data_bits, Double, Int);
    try_read_3(parity, Double, Int);
    try_read_3(stop_bits, Double, Int);
    return {value};
}

auto FilePath::Uart::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["reserved"] = static_cast<int>(reserved);
    value["baud_rate"] = toHex(baud_rate);
    value["data_bits"] = static_cast<int>(data_bits);
    value["parity"] = static_cast<int>(parity);
    value["stop_bits"] = static_cast<int>(stop_bits);
    return value;
}

auto FilePath::Uart::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Uart(%1,%2,%3,%4)").arg(baud_rate).arg(data_bits).arg(static_cast<int>(parity)).arg(static_cast<int>(stop_bits));
}

FilePath::UsbClass::UsbClass(const EFIBoot::File_path::MSG::Usb_class &_usb_class)
    : _string{}
    , vendor_id{_usb_class.vendor_id}
    , product_id{_usb_class.product_id}
    , device_class{_usb_class.device_class}
    , device_subclass{_usb_class.device_subclass}
    , device_protocol{_usb_class.device_protocol}
{
}

auto FilePath::UsbClass::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Usb_class
{
    EFIBoot::File_path::MSG::Usb_class value{};
    value.vendor_id = vendor_id;
    value.product_id = product_id;
    value.device_class = device_class;
    value.device_subclass = device_subclass;
    value.device_protocol = device_protocol;
    return value;
}

auto FilePath::UsbClass::fromJSON(const QJsonObject &obj) -> std::optional<UsbClass>
{
    UsbClass value{};
    check_obj();
    try_read_3(vendor_id, Double, Int);
    try_read_3(product_id, Double, Int);
    try_read_3(device_class, Double, Int);
    try_read_3(device_subclass, Double, Int);
    try_read_3(device_protocol, Double, Int);
    return {value};
}

auto FilePath::UsbClass::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["vendor_id"] = static_cast<int>(vendor_id);
    value["product_id"] = static_cast<int>(product_id);
    value["device_class"] = static_cast<int>(device_class);
    value["device_subclass"] = static_cast<int>(device_subclass);
    value["device_protocol"] = static_cast<int>(device_protocol);
    return value;
}

auto FilePath::UsbClass::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("UsbClass(%1,%2,%3,%4,%5)").arg(vendor_id).arg(product_id).arg(device_class).arg(device_subclass).arg(device_protocol);
}

FilePath::UsbWwid::UsbWwid(const EFIBoot::File_path::MSG::Usb_wwid &_usb_wwid)
    : _string{}
    , interface_number{_usb_wwid.interface_number}
    , device_vendor_id{_usb_wwid.device_vendor_id}
    , device_product_id{_usb_wwid.device_product_id}
    , serial_number{QString::fromStdU16String(_usb_wwid.serial_number)}
{
}

auto FilePath::UsbWwid::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Usb_wwid
{
    EFIBoot::File_path::MSG::Usb_wwid value{};
    value.interface_number = interface_number;
    value.device_vendor_id = device_vendor_id;
    value.device_product_id = device_product_id;
    value.serial_number = serial_number.toStdU16String();
    return value;
}

auto FilePath::UsbWwid::fromJSON(const QJsonObject &obj) -> std::optional<UsbWwid>
{
    UsbWwid value{};
    check_obj();
    try_read_3(interface_number, Double, Int);
    try_read_3(device_vendor_id, Double, Int);
    try_read_3(device_product_id, Double, Int);
    try_read(serial_number, String);
    return {value};
}

auto FilePath::UsbWwid::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["interface_number"] = static_cast<int>(interface_number);
    value["device_vendor_id"] = static_cast<int>(device_vendor_id);
    value["device_product_id"] = static_cast<int>(device_product_id);
    value["serial_number"] = serial_number;
    return value;
}

auto FilePath::UsbWwid::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("UsbWwid(%1,%2,%3)").arg(device_vendor_id).arg(device_product_id).arg(interface_number);
}

FilePath::DeviceLogicalUnit::DeviceLogicalUnit(const EFIBoot::File_path::MSG::Device_logical_unit &_device_logical_unit)
    : _string{}
    , lun{_device_logical_unit.lun}
{
}

auto FilePath::DeviceLogicalUnit::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Device_logical_unit
{
    EFIBoot::File_path::MSG::Device_logical_unit value{};
    value.lun = lun;
    return value;
}

auto FilePath::DeviceLogicalUnit::fromJSON(const QJsonObject &obj) -> std::optional<DeviceLogicalUnit>
{
    DeviceLogicalUnit value{};
    check_obj();
    try_read_3(lun, Double, Int);
    return {value};
}

auto FilePath::DeviceLogicalUnit::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["lun"] = static_cast<int>(lun);
    return value;
}

auto FilePath::DeviceLogicalUnit::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Unit(%1)").arg(lun);
}

FilePath::Sata::Sata(const EFIBoot::File_path::MSG::Sata &_sata)
    : _string{}
    , hba_port_number{_sata.hba_port_number}
    , port_multiplier_port_number{_sata.port_multiplier_port_number}
    , lun{_sata.lun}
{
}

auto FilePath::Sata::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Sata
{
    EFIBoot::File_path::MSG::Sata value{};
    value.hba_port_number = hba_port_number;
    value.port_multiplier_port_number = port_multiplier_port_number;
    value.lun = lun;
    return value;
}

auto FilePath::Sata::fromJSON(const QJsonObject &obj) -> std::optional<Sata>
{
    Sata value{};
    check_obj();
    // Support for old names
    try_read_4(hba_port_number, "hba_port", Double, Int);
    try_read_4(port_multiplier_port_number, "port_multiplier_port", Double, Int);
    try_read_3(lun, Double, Int);
    return {value};
}

auto FilePath::Sata::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["hba_port_number"] = static_cast<int>(hba_port_number);
    value["port_multiplier_port_number"] = static_cast<int>(port_multiplier_port_number);
    value["lun"] = static_cast<int>(lun);
    return value;
}

auto FilePath::Sata::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Sata(%1,%2,%3)").arg(hba_port_number).arg(port_multiplier_port_number).arg(lun);
}

FilePath::Iscsi::Iscsi(const EFIBoot::File_path::MSG::Iscsi &_iscsi)
    : _string{}
    , protocol{_iscsi.protocol}
    , options{_iscsi.options}
    , lun{_iscsi.lun}
    , target_portal_group{_iscsi.target_portal_group}
    , target_name{QByteArray::fromRawData(_iscsi.target_name.data(), static_cast<int>(_iscsi.target_name.size() * sizeof(decltype(_iscsi.target_name)::value_type)))}
{
}

auto FilePath::Iscsi::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Iscsi
{
    EFIBoot::File_path::MSG::Iscsi value{};
    value.protocol = protocol;
    value.options = options;
    value.lun = lun;
    value.target_portal_group = target_portal_group;
    value.target_name = target_name.toStdString();
    return value;
}

auto FilePath::Iscsi::fromJSON(const QJsonObject &obj) -> std::optional<Iscsi>
{
    Iscsi value{};
    check_obj();
    try_read_3(protocol, Double, Int);
    try_read_3(options, Double, Int);
    check_type(lun, String);
    value.lun = obj["lun"].toString().toULongLong(nullptr, HEX_BASE);
    try_read_3(target_portal_group, Double, Int);
    try_read(target_name, String);
    return {value};
}

auto FilePath::Iscsi::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["protocol"] = static_cast<int>(protocol);
    value["options"] = static_cast<int>(options);
    value["lun"] = toHex(lun);
    value["target_portal_group"] = static_cast<int>(target_portal_group);
    value["target_name"] = target_name;
    return value;
}

auto FilePath::Iscsi::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("iSCSI(%1,%2,%3,%4,%5)")
                         .arg(target_name)
                         .arg(toHex(target_portal_group))
                         .arg(toHex(lun))
                         .arg(toHex(options))
                         .arg(protocol == 0 ? "TCP" : QString::number(protocol));
}

FilePath::Vlan::Vlan(const EFIBoot::File_path::MSG::Vlan &_vlan)
    : _string{}
    , vlan_id{_vlan.vlan_id}
{
}

auto FilePath::Vlan::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Vlan
{
    EFIBoot::File_path::MSG::Vlan value{};
    value.vlan_id = vlan_id;
    return value;
}

auto FilePath::Vlan::fromJSON(const QJsonObject &obj) -> std::optional<Vlan>
{
    Vlan value{};
    check_obj();
    try_read_3(vlan_id, Double, Int);
    return {value};
}

auto FilePath::Vlan::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["vlan_id"] = static_cast<int>(vlan_id);
    return value;
}

auto FilePath::Vlan::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Vlan(%1)").arg(vlan_id);
}

FilePath::FibreChannelEx::FibreChannelEx(const EFIBoot::File_path::MSG::Fibre_channel_ex &_fibre_channel_ex)
    : _string{}
    , reserved{_fibre_channel_ex.reserved}
    , world_wide_name{_fibre_channel_ex.world_wide_name}
    , lun{_fibre_channel_ex.lun}
{
}

auto FilePath::FibreChannelEx::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Fibre_channel_ex
{
    EFIBoot::File_path::MSG::Fibre_channel_ex value{};
    value.reserved = reserved;
    value.world_wide_name = world_wide_name;
    value.lun = lun;
    return value;
}

auto FilePath::FibreChannelEx::fromJSON(const QJsonObject &obj) -> std::optional<FibreChannelEx>
{
    FibreChannelEx value{};
    check_obj();
    try_read_3(reserved, Double, Int);
    check_type(world_wide_name, String);
    value.world_wide_name = obj["world_wide_name"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(lun, String);
    value.lun = obj["lun"].toString().toULongLong(nullptr, HEX_BASE);
    return {value};
}

auto FilePath::FibreChannelEx::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["reserved"] = static_cast<int>(reserved);
    value["world_wide_name"] = toHex(world_wide_name);
    value["lun"] = toHex(lun);
    return value;
}

auto FilePath::FibreChannelEx::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("FibreEx(%1,%2)").arg(world_wide_name).arg(lun);
}

FilePath::SasExtendedMessaging::SasExtendedMessaging(const EFIBoot::File_path::MSG::Sas_extended_messaging &_sas_extended_messaging)
    : _string{}
    , sas_address{_sas_extended_messaging.sas_address}
    , lun{_sas_extended_messaging.lun}
    , device_and_topology_info{_sas_extended_messaging.device_and_topology_info}
    , relative_target_port{_sas_extended_messaging.relative_target_port}
{
}

auto FilePath::SasExtendedMessaging::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Sas_extended_messaging
{
    EFIBoot::File_path::MSG::Sas_extended_messaging value{};
    value.sas_address = sas_address;
    value.lun = lun;
    value.device_and_topology_info = device_and_topology_info;
    value.relative_target_port = relative_target_port;
    return value;
}

auto FilePath::SasExtendedMessaging::fromJSON(const QJsonObject &obj) -> std::optional<SasExtendedMessaging>
{
    SasExtendedMessaging value{};
    check_obj();
    check_type(sas_address, String);
    value.sas_address = obj["sas_address"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(lun, String);
    value.lun = obj["lun"].toString().toULongLong(nullptr, HEX_BASE);
    try_read_3(device_and_topology_info, Double, Int);
    try_read_3(relative_target_port, Double, Int);
    return {value};
}

auto FilePath::SasExtendedMessaging::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["sas_address"] = toHex(sas_address);
    value["lun"] = toHex(lun);
    value["device_and_topology_info"] = static_cast<int>(device_and_topology_info);
    value["relative_target_port"] = static_cast<int>(relative_target_port);
    return value;
}

auto FilePath::SasExtendedMessaging::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("SasEx(%1,%2,%3,%4)").arg(sas_address).arg(lun).arg(relative_target_port).arg(device_and_topology_info);
}

FilePath::NvmExpressNs::NvmExpressNs(const EFIBoot::File_path::MSG::Nvm_express_ns &_nvm_express_ns)
    : _string{}
    , namespace_identifier{_nvm_express_ns.namespace_identifier}
    , ieee_extended_unique_identifier{_nvm_express_ns.ieee_extended_unique_identifier}
{
}

auto FilePath::NvmExpressNs::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Nvm_express_ns
{
    EFIBoot::File_path::MSG::Nvm_express_ns value{};
    value.namespace_identifier = namespace_identifier;
    value.ieee_extended_unique_identifier = ieee_extended_unique_identifier;
    return value;
}

auto FilePath::NvmExpressNs::fromJSON(const QJsonObject &obj) -> std::optional<NvmExpressNs>
{
    NvmExpressNs value{};
    check_obj();
    try_read_3(namespace_identifier, Double, Int);
    check_type(ieee_extended_unique_identifier, String);
    value.ieee_extended_unique_identifier = obj["ieee_extended_unique_identifier"].toString().toULongLong(nullptr, HEX_BASE);
    return {value};
}

auto FilePath::NvmExpressNs::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["namespace_identifier"] = static_cast<int>(namespace_identifier);
    value["ieee_extended_unique_identifier"] = toHex(ieee_extended_unique_identifier);
    return value;
}

auto FilePath::NvmExpressNs::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("NVMe(%1,%2)").arg(toHex(namespace_identifier)).arg(toHex(ieee_extended_unique_identifier));
}

FilePath::Uri::Uri(const EFIBoot::File_path::MSG::Uri &_uri)
    : _string{}
    , uri{QUrl::fromEncoded(QByteArray::fromRawData(reinterpret_cast<const char *>(_uri.uri.data()), static_cast<int>(_uri.uri.size() * sizeof(decltype(_uri.uri)::value_type))))}
{
}

auto FilePath::Uri::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Uri
{
    EFIBoot::File_path::MSG::Uri value{};
    {
        auto encoded = uri.toEncoded();
        value.uri = {encoded.begin(), encoded.end()};
    }
    return value;
}

auto FilePath::Uri::fromJSON(const QJsonObject &obj) -> std::optional<Uri>
{
    Uri value{};
    check_obj();
    check_type(uri, String);
    value.uri = QUrl::fromEncoded(obj["uri"].toString().toUtf8());
    return {value};
}

auto FilePath::Uri::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["uri"] = static_cast<QString>(uri.toEncoded());
    return value;
}

auto FilePath::Uri::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Uri(%1)").arg(uri.toDisplayString());
}

FilePath::Ufs::Ufs(const EFIBoot::File_path::MSG::Ufs &_ufs)
    : _string{}
    , pun{_ufs.pun}
    , lun{_ufs.lun}
{
}

auto FilePath::Ufs::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Ufs
{
    EFIBoot::File_path::MSG::Ufs value{};
    value.pun = pun;
    value.lun = lun;
    return value;
}

auto FilePath::Ufs::fromJSON(const QJsonObject &obj) -> std::optional<Ufs>
{
    Ufs value{};
    check_obj();
    try_read_3(pun, Double, Int);
    try_read_3(lun, Double, Int);
    return {value};
}

auto FilePath::Ufs::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["pun"] = static_cast<int>(pun);
    value["lun"] = static_cast<int>(lun);
    return value;
}

auto FilePath::Ufs::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("UFS(%1,%2)").arg(pun).arg(lun);
}

FilePath::Sd::Sd(const EFIBoot::File_path::MSG::Sd &_sd)
    : _string{}
    , slot_number{_sd.slot_number}
{
}

auto FilePath::Sd::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Sd
{
    EFIBoot::File_path::MSG::Sd value{};
    value.slot_number = slot_number;
    return value;
}

auto FilePath::Sd::fromJSON(const QJsonObject &obj) -> std::optional<Sd>
{
    Sd value{};
    check_obj();
    try_read_3(slot_number, Double, Int);
    return {value};
}

auto FilePath::Sd::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["slot_number"] = static_cast<int>(slot_number);
    return value;
}

auto FilePath::Sd::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("SD(%1)").arg(slot_number);
}

FilePath::Bluetooth::Bluetooth(const EFIBoot::File_path::MSG::Bluetooth &_bluetooth)
    : _string{}
    , device_address{QByteArray::fromRawData(reinterpret_cast<const char *>(_bluetooth.device_address.data()), static_cast<int>(_bluetooth.device_address.size() * sizeof(decltype(_bluetooth.device_address)::value_type)))}
{
}

auto FilePath::Bluetooth::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Bluetooth
{
    EFIBoot::File_path::MSG::Bluetooth value{};
    {
        auto bytes = QByteArray::fromHex(device_address.toUtf8());
        memcpy(value.device_address.data(), bytes.data(), qMin(static_cast<size_t>(bytes.size()), sizeof(value.device_address)));
    }
    return value;
}

auto FilePath::Bluetooth::fromJSON(const QJsonObject &obj) -> std::optional<Bluetooth>
{
    Bluetooth value{};
    check_obj();
    try_read(device_address, String);
    return {value};
}

auto FilePath::Bluetooth::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["device_address"] = device_address;
    return value;
}

auto FilePath::Bluetooth::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Bluetooth(%1)").arg(device_address);
}

FilePath::WiFi::WiFi(const EFIBoot::File_path::MSG::Wi_fi &_wi_fi)
    : _string{}
    , ssid{QByteArray::fromRawData(_wi_fi.ssid.data(), static_cast<int>(_wi_fi.ssid.size() * sizeof(decltype(_wi_fi.ssid)::value_type)))}
{
}

auto FilePath::WiFi::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Wi_fi
{
    EFIBoot::File_path::MSG::Wi_fi value{};
    value.ssid = ssid.toStdString();
    return value;
}

auto FilePath::WiFi::fromJSON(const QJsonObject &obj) -> std::optional<WiFi>
{
    WiFi value{};
    check_obj();
    try_read(ssid, String);
    return {value};
}

auto FilePath::WiFi::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["ssid"] = ssid;
    return value;
}

auto FilePath::WiFi::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Wi-Fi(%1)").arg(ssid);
}

FilePath::Emmc::Emmc(const EFIBoot::File_path::MSG::Emmc &_emmc)
    : _string{}
    , slot_number{_emmc.slot_number}
{
}

auto FilePath::Emmc::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Emmc
{
    EFIBoot::File_path::MSG::Emmc value{};
    value.slot_number = slot_number;
    return value;
}

auto FilePath::Emmc::fromJSON(const QJsonObject &obj) -> std::optional<Emmc>
{
    Emmc value{};
    check_obj();
    try_read_3(slot_number, Double, Int);
    return {value};
}

auto FilePath::Emmc::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["slot_number"] = static_cast<int>(slot_number);
    return value;
}

auto FilePath::Emmc::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("eMMC(%1)").arg(slot_number);
}

FilePath::Bluetoothle::Bluetoothle(const EFIBoot::File_path::MSG::Bluetoothle &_bluetoothle)
    : _string{}
    , device_address{QByteArray::fromRawData(reinterpret_cast<const char *>(_bluetoothle.device_address.data()), static_cast<int>(_bluetoothle.device_address.size() * sizeof(decltype(_bluetoothle.device_address)::value_type)))}
    , address_type{_bluetoothle.address_type}
{
}

auto FilePath::Bluetoothle::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Bluetoothle
{
    EFIBoot::File_path::MSG::Bluetoothle value{};
    {
        auto bytes = QByteArray::fromHex(device_address.toUtf8());
        memcpy(value.device_address.data(), bytes.data(), qMin(static_cast<size_t>(bytes.size()), sizeof(value.device_address)));
    }
    value.address_type = address_type;
    return value;
}

auto FilePath::Bluetoothle::fromJSON(const QJsonObject &obj) -> std::optional<Bluetoothle>
{
    Bluetoothle value{};
    check_obj();
    try_read(device_address, String);
    try_read_3(address_type, Double, Int);
    return {value};
}

auto FilePath::Bluetoothle::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["device_address"] = device_address;
    value["address_type"] = static_cast<int>(address_type);
    return value;
}

auto FilePath::Bluetoothle::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("BluetoothLE(%1,%2)").arg(device_address).arg(static_cast<int>(address_type));
}

FilePath::Dns::Dns(const EFIBoot::File_path::MSG::Dns &_dns)
    : _string{}
    , ipv6{_dns.ipv6}
    , data{QByteArray::fromRawData(reinterpret_cast<const char *>(_dns.data.data()), static_cast<int>(_dns.data.size() * sizeof(decltype(_dns.data)::value_type)))}
{
    data.detach();
}

auto FilePath::Dns::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Dns
{
    EFIBoot::File_path::MSG::Dns value{};
    value.ipv6 = ipv6;
    value.data = {data.begin(), data.end()};
    return value;
}

auto FilePath::Dns::fromJSON(const QJsonObject &obj) -> std::optional<Dns>
{
    Dns value{};
    check_obj();
    try_read(ipv6, Bool);
    check_type(data, String);
    value.data = QByteArray::fromBase64(obj["data"].toString().toUtf8());
    return {value};
}

auto FilePath::Dns::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["ipv6"] = ipv6;
    value["data"] = static_cast<QString>(data.toBase64());
    return value;
}

auto FilePath::Dns::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("DNS([%1B])").arg(data.size());
}

FilePath::NvdimmNs::NvdimmNs(const EFIBoot::File_path::MSG::Nvdimm_ns &_nvdimm_ns)
    : _string{}
    , uuid{}
{
    static_assert(sizeof(uuid) == sizeof(_nvdimm_ns.uuid));
    memcpy(reinterpret_cast<void *>(&uuid), &_nvdimm_ns.uuid, sizeof(uuid));
}

auto FilePath::NvdimmNs::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Nvdimm_ns
{
    EFIBoot::File_path::MSG::Nvdimm_ns value{};
    static_assert(sizeof(uuid) == sizeof(value.uuid));
    memcpy(value.uuid.data(), &uuid, sizeof(value.uuid));
    return value;
}

auto FilePath::NvdimmNs::fromJSON(const QJsonObject &obj) -> std::optional<NvdimmNs>
{
    NvdimmNs value{};
    check_obj();
    check_type(uuid, String);
    value.uuid = QUuid::fromString(obj["uuid"].toString());
    return {value};
}

auto FilePath::NvdimmNs::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["uuid"] = uuid.toString();
    return value;
}

auto FilePath::NvdimmNs::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Nvdimm(%1)").arg(uuid.toString(QUuid::WithoutBraces));
}

FilePath::RestService::RestService(const EFIBoot::File_path::MSG::Rest_service &_rest_service)
    : _string{}
    , rest_service{_rest_service.rest_service}
    , access_mode{_rest_service.access_mode}
    , guid{}
    , data{QByteArray::fromRawData(reinterpret_cast<const char *>(_rest_service.data.data()), static_cast<int>(_rest_service.data.size() * sizeof(decltype(_rest_service.data)::value_type)))}
{
    static_assert(sizeof(guid) == sizeof(_rest_service.guid));
    memcpy(reinterpret_cast<void *>(&guid), &_rest_service.guid, sizeof(guid));
    data.detach();
}

auto FilePath::RestService::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Rest_service
{
    EFIBoot::File_path::MSG::Rest_service value{};
    value.rest_service = rest_service;
    value.access_mode = access_mode;
    static_assert(sizeof(guid) == sizeof(value.guid));
    memcpy(value.guid.data(), &guid, sizeof(value.guid));
    value.data = {data.begin(), data.end()};
    return value;
}

auto FilePath::RestService::fromJSON(const QJsonObject &obj) -> std::optional<RestService>
{
    RestService value{};
    check_obj();
    try_read_3(rest_service, Double, Int);
    try_read_3(access_mode, Double, Int);
    check_type(guid, String);
    value.guid = QUuid::fromString(obj["guid"].toString());
    check_type(data, String);
    value.data = QByteArray::fromBase64(obj["data"].toString().toUtf8());
    return {value};
}

auto FilePath::RestService::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["rest_service"] = static_cast<int>(rest_service);
    value["access_mode"] = static_cast<int>(access_mode);
    value["guid"] = guid.toString();
    value["data"] = static_cast<QString>(data.toBase64());
    return value;
}

auto FilePath::RestService::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("RestService(%1,%2,%3,[%4B])").arg(static_cast<int>(rest_service)).arg(static_cast<int>(access_mode)).arg(guid.toString(QUuid::WithoutBraces)).arg(data.size());
}

FilePath::NvmeOfNs::NvmeOfNs(const EFIBoot::File_path::MSG::Nvme_of_ns &_nvme_of_ns)
    : _string{}
    , nidt{_nvme_of_ns.nidt}
    , nid{}
    , subsystem_nqn{QByteArray::fromRawData(_nvme_of_ns.subsystem_nqn.data(), static_cast<int>(_nvme_of_ns.subsystem_nqn.size() * sizeof(decltype(_nvme_of_ns.subsystem_nqn)::value_type)))}
{
    static_assert(sizeof(nid) == sizeof(_nvme_of_ns.nid));
    memcpy(reinterpret_cast<void *>(&nid), &_nvme_of_ns.nid, sizeof(nid));
}

auto FilePath::NvmeOfNs::toEFIBootFilePath() const -> EFIBoot::File_path::MSG::Nvme_of_ns
{
    EFIBoot::File_path::MSG::Nvme_of_ns value{};
    value.nidt = nidt;
    static_assert(sizeof(nid) == sizeof(value.nid));
    memcpy(value.nid.data(), &nid, sizeof(value.nid));
    value.subsystem_nqn = subsystem_nqn.toStdString();
    return value;
}

auto FilePath::NvmeOfNs::fromJSON(const QJsonObject &obj) -> std::optional<NvmeOfNs>
{
    NvmeOfNs value{};
    check_obj();
    try_read_3(nidt, Double, Int);
    check_type(nid, String);
    value.nid = QUuid::fromString(obj["nid"].toString());
    try_read(subsystem_nqn, String);
    return {value};
}

auto FilePath::NvmeOfNs::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["nidt"] = static_cast<int>(nidt);
    value["nid"] = nid.toString();
    value["subsystem_nqn"] = subsystem_nqn;
    return value;
}

auto FilePath::NvmeOfNs::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("NVMeoF([%1B],%2)").arg(subsystem_nqn.size()).arg(nid.toString(QUuid::WithoutBraces));
}

// Media

FilePath::Hd::Hd(const EFIBoot::File_path::MEDIA::Hd &_hd)
    : _string{}
    , partition_number{_hd.partition_number}
    , partition_start{_hd.partition_start}
    , partition_size{_hd.partition_size}
    , partition_signature{}
    , partition_format{_hd.partition_format}
    , signature_type{_hd.signature_type}
{
    static_assert(sizeof(partition_signature) == sizeof(_hd.partition_signature));
    memcpy(reinterpret_cast<void *>(&partition_signature), &_hd.partition_signature, sizeof(partition_signature));
}

auto FilePath::Hd::toEFIBootFilePath() const -> EFIBoot::File_path::MEDIA::Hd
{
    EFIBoot::File_path::MEDIA::Hd value{};
    value.partition_number = partition_number;
    value.partition_start = partition_start;
    value.partition_size = partition_size;
    static_assert(sizeof(partition_signature) == sizeof(value.partition_signature));
    memcpy(value.partition_signature.data(), &partition_signature, sizeof(value.partition_signature));
    value.partition_format = partition_format;
    value.signature_type = signature_type;
    return value;
}

auto FilePath::Hd::fromJSON(const QJsonObject &obj) -> std::optional<Hd>
{
    Hd value{};
    check_obj();
    try_read_3(partition_number, Double, Int);
    check_type(partition_start, String);
    value.partition_start = obj["partition_start"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(partition_size, String);
    value.partition_size = obj["partition_size"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(partition_signature, String);
    value.partition_signature = QUuid::fromString(obj["partition_signature"].toString());
    try_read_3(partition_format, Double, Int);
    try_read_3(signature_type, Double, Int);
    return {value};
}

auto FilePath::Hd::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["partition_number"] = static_cast<int>(partition_number);
    value["partition_start"] = toHex(partition_start);
    value["partition_size"] = toHex(partition_size);
    value["partition_signature"] = partition_signature.toString();
    value["partition_format"] = static_cast<int>(partition_format);
    value["signature_type"] = static_cast<int>(signature_type);
    return value;
}

auto FilePath::Hd::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("HD(%1,%2,%3,%4,%5)")
                         .arg(partition_number)
                         .arg(signature_type == EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE::MBR ? "MBR" : (signature_type == EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE::GUID ? "GPT" : QString::number(static_cast<int>(signature_type))))
                         .arg(signature_type == EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE::MBR ? toHex(partition_signature.data1) : (signature_type == EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE::GUID ? partition_signature.toString(QUuid::WithoutBraces) : "N/A"))
                         .arg(toHex(partition_start))
                         .arg(toHex(partition_size));
}

FilePath::CdRom::CdRom(const EFIBoot::File_path::MEDIA::Cd_rom &_cd_rom)
    : _string{}
    , boot_entry{_cd_rom.boot_entry}
    , partition_start{_cd_rom.partition_start}
    , partition_size{_cd_rom.partition_size}
{
}

auto FilePath::CdRom::toEFIBootFilePath() const -> EFIBoot::File_path::MEDIA::Cd_rom
{
    EFIBoot::File_path::MEDIA::Cd_rom value{};
    value.boot_entry = boot_entry;
    value.partition_start = partition_start;
    value.partition_size = partition_size;
    return value;
}

auto FilePath::CdRom::fromJSON(const QJsonObject &obj) -> std::optional<CdRom>
{
    CdRom value{};
    check_obj();
    try_read_3(boot_entry, Double, Int);
    check_type(partition_start, String);
    value.partition_start = obj["partition_start"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(partition_size, String);
    value.partition_size = obj["partition_size"].toString().toULongLong(nullptr, HEX_BASE);
    return {value};
}

auto FilePath::CdRom::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["boot_entry"] = static_cast<int>(boot_entry);
    value["partition_start"] = toHex(partition_start);
    value["partition_size"] = toHex(partition_size);
    return value;
}

auto FilePath::CdRom::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("CDROM(%1,%2,%3)").arg(boot_entry).arg(partition_start).arg(partition_size);
}

FilePath::FilePath::FilePath(const EFIBoot::File_path::MEDIA::File_path &_file_path)
    : _string{}
    , path_name{QString::fromStdU16String(_file_path.path_name)}
{
}

auto FilePath::FilePath::toEFIBootFilePath() const -> EFIBoot::File_path::MEDIA::File_path
{
    EFIBoot::File_path::MEDIA::File_path value{};
    value.path_name = path_name.toStdU16String();
    return value;
}

auto FilePath::FilePath::fromJSON(const QJsonObject &obj) -> std::optional<FilePath>
{
    FilePath value{};
    if(obj["type"] != TYPE)
        return std::nullopt;

    // Support for old names
    if(obj["subtype"] != SUBTYPE && obj["subtype"] != "FILE")
        return std::nullopt;

    // check_obj();
    try_read_4(path_name, "name", String, String);
    return {value};
}

auto FilePath::FilePath::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["path_name"] = path_name;
    return value;
}

auto FilePath::FilePath::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = path_name;
}

FilePath::Protocol::Protocol(const EFIBoot::File_path::MEDIA::Protocol &_protocol)
    : _string{}
    , guid{}
{
    static_assert(sizeof(guid) == sizeof(_protocol.guid));
    memcpy(reinterpret_cast<void *>(&guid), &_protocol.guid, sizeof(guid));
}

auto FilePath::Protocol::toEFIBootFilePath() const -> EFIBoot::File_path::MEDIA::Protocol
{
    EFIBoot::File_path::MEDIA::Protocol value{};
    static_assert(sizeof(guid) == sizeof(value.guid));
    memcpy(value.guid.data(), &guid, sizeof(value.guid));
    return value;
}

auto FilePath::Protocol::fromJSON(const QJsonObject &obj) -> std::optional<Protocol>
{
    Protocol value{};
    check_obj();
    check_type(guid, String);
    value.guid = QUuid::fromString(obj["guid"].toString());
    return {value};
}

auto FilePath::Protocol::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["guid"] = guid.toString();
    return value;
}

auto FilePath::Protocol::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Media(%1)").arg(guid.toString(QUuid::WithoutBraces));
}

FilePath::FirmwareFile::FirmwareFile(const EFIBoot::File_path::MEDIA::Firmware_file &_firmware_file)
    : _string{}
    , name{}
{
    static_assert(sizeof(name) == sizeof(_firmware_file.name));
    memcpy(reinterpret_cast<void *>(&name), &_firmware_file.name, sizeof(name));
}

auto FilePath::FirmwareFile::toEFIBootFilePath() const -> EFIBoot::File_path::MEDIA::Firmware_file
{
    EFIBoot::File_path::MEDIA::Firmware_file value{};
    static_assert(sizeof(name) == sizeof(value.name));
    memcpy(value.name.data(), &name, sizeof(value.name));
    return value;
}

auto FilePath::FirmwareFile::fromJSON(const QJsonObject &obj) -> std::optional<FirmwareFile>
{
    FirmwareFile value{};
    check_obj();
    check_type(name, String);
    value.name = QUuid::fromString(obj["name"].toString());
    return {value};
}

auto FilePath::FirmwareFile::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["name"] = name.toString();
    return value;
}

auto FilePath::FirmwareFile::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("FvFile(%1)").arg(name.toString(QUuid::WithoutBraces));
}

FilePath::FirmwareVolume::FirmwareVolume(const EFIBoot::File_path::MEDIA::Firmware_volume &_firmware_volume)
    : _string{}
    , name{}
{
    static_assert(sizeof(name) == sizeof(_firmware_volume.name));
    memcpy(reinterpret_cast<void *>(&name), &_firmware_volume.name, sizeof(name));
}

auto FilePath::FirmwareVolume::toEFIBootFilePath() const -> EFIBoot::File_path::MEDIA::Firmware_volume
{
    EFIBoot::File_path::MEDIA::Firmware_volume value{};
    static_assert(sizeof(name) == sizeof(value.name));
    memcpy(value.name.data(), &name, sizeof(value.name));
    return value;
}

auto FilePath::FirmwareVolume::fromJSON(const QJsonObject &obj) -> std::optional<FirmwareVolume>
{
    FirmwareVolume value{};
    check_obj();
    check_type(name, String);
    value.name = QUuid::fromString(obj["name"].toString());
    return {value};
}

auto FilePath::FirmwareVolume::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["name"] = name.toString();
    return value;
}

auto FilePath::FirmwareVolume::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Fv(%1)").arg(name.toString(QUuid::WithoutBraces));
}

FilePath::RelativeOffsetRange::RelativeOffsetRange(const EFIBoot::File_path::MEDIA::Relative_offset_range &_relative_offset_range)
    : _string{}
    , reserved{_relative_offset_range.reserved}
    , starting_offset{_relative_offset_range.starting_offset}
    , ending_offset{_relative_offset_range.ending_offset}
{
}

auto FilePath::RelativeOffsetRange::toEFIBootFilePath() const -> EFIBoot::File_path::MEDIA::Relative_offset_range
{
    EFIBoot::File_path::MEDIA::Relative_offset_range value{};
    value.reserved = reserved;
    value.starting_offset = starting_offset;
    value.ending_offset = ending_offset;
    return value;
}

auto FilePath::RelativeOffsetRange::fromJSON(const QJsonObject &obj) -> std::optional<RelativeOffsetRange>
{
    RelativeOffsetRange value{};
    check_obj();
    try_read_3(reserved, Double, Int);
    check_type(starting_offset, String);
    value.starting_offset = obj["starting_offset"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(ending_offset, String);
    value.ending_offset = obj["ending_offset"].toString().toULongLong(nullptr, HEX_BASE);
    return {value};
}

auto FilePath::RelativeOffsetRange::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["reserved"] = static_cast<int>(reserved);
    value["starting_offset"] = toHex(starting_offset);
    value["ending_offset"] = toHex(ending_offset);
    return value;
}

auto FilePath::RelativeOffsetRange::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Offset(%1,%2)").arg(starting_offset).arg(ending_offset);
}

FilePath::RamDisk::RamDisk(const EFIBoot::File_path::MEDIA::Ram_disk &_ram_disk)
    : _string{}
    , starting_address{_ram_disk.starting_address}
    , ending_address{_ram_disk.ending_address}
    , guid{}
    , disk_instance{_ram_disk.disk_instance}
{
    static_assert(sizeof(guid) == sizeof(_ram_disk.guid));
    memcpy(reinterpret_cast<void *>(&guid), &_ram_disk.guid, sizeof(guid));
}

auto FilePath::RamDisk::toEFIBootFilePath() const -> EFIBoot::File_path::MEDIA::Ram_disk
{
    EFIBoot::File_path::MEDIA::Ram_disk value{};
    value.starting_address = starting_address;
    value.ending_address = ending_address;
    static_assert(sizeof(guid) == sizeof(value.guid));
    memcpy(value.guid.data(), &guid, sizeof(value.guid));
    value.disk_instance = disk_instance;
    return value;
}

auto FilePath::RamDisk::fromJSON(const QJsonObject &obj) -> std::optional<RamDisk>
{
    RamDisk value{};
    check_obj();
    check_type(starting_address, String);
    value.starting_address = obj["starting_address"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(ending_address, String);
    value.ending_address = obj["ending_address"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(guid, String);
    value.guid = QUuid::fromString(obj["guid"].toString());
    try_read_3(disk_instance, Double, Int);
    return {value};
}

auto FilePath::RamDisk::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["starting_address"] = toHex(starting_address);
    value["ending_address"] = toHex(ending_address);
    value["guid"] = guid.toString();
    value["disk_instance"] = static_cast<int>(disk_instance);
    return value;
}

auto FilePath::RamDisk::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("RamDisk(%1,%2,%3,%4)").arg(starting_address).arg(ending_address).arg(disk_instance).arg(guid.toString(QUuid::WithoutBraces));
}

// BIOS

FilePath::BootSpecification::BootSpecification(const EFIBoot::File_path::BIOS::Boot_specification &_boot_specification)
    : _string{}
    , device_type{_boot_specification.device_type}
    , status_flag{_boot_specification.status_flag}
    , description{QByteArray::fromRawData(_boot_specification.description.data(), static_cast<int>(_boot_specification.description.size() * sizeof(decltype(_boot_specification.description)::value_type)))}
{
}

auto FilePath::BootSpecification::toEFIBootFilePath() const -> EFIBoot::File_path::BIOS::Boot_specification
{
    EFIBoot::File_path::BIOS::Boot_specification value{};
    value.device_type = device_type;
    value.status_flag = status_flag;
    value.description = description.toStdString();
    return value;
}

auto FilePath::BootSpecification::fromJSON(const QJsonObject &obj) -> std::optional<BootSpecification>
{
    BootSpecification value{};
    if(obj["type"] != TYPE)
        return std::nullopt;

    // Support for old names
    if(obj["subtype"] != SUBTYPE && obj["subtype"] != "BIOS_BOOT_SPECIFICATION")
        return std::nullopt;

    // check_obj();
    try_read_3(device_type, Double, Int);
    try_read_3(status_flag, Double, Int);
    try_read(description, String);
    return {value};
}

auto FilePath::BootSpecification::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["device_type"] = static_cast<int>(device_type);
    value["status_flag"] = static_cast<int>(status_flag);
    value["description"] = description;
    return value;
}

auto FilePath::BootSpecification::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("BBS(%1,%2,%3)").arg(toHex(device_type), description, toHex(status_flag));
}

FilePath::Vendor::Vendor(const EFIBoot::File_path::HW::Vendor &vendor)
    : data{QByteArray::fromRawData(reinterpret_cast<const char *>(vendor.data.data()), static_cast<int>(vendor.data.size() * sizeof(decltype(vendor.data)::value_type)))}
    , _type{EFIBoot::File_path::HW::Vendor::TYPE}
{
    data.detach();
    static_assert(sizeof(guid) == sizeof(vendor.guid));
    memcpy(reinterpret_cast<void *>(&guid), &vendor.guid, sizeof(vendor.guid));
}

FilePath::Vendor::Vendor(const EFIBoot::File_path::MSG::Vendor &vendor)
    : data{QByteArray::fromRawData(reinterpret_cast<const char *>(vendor.data.data()), static_cast<int>(vendor.data.size() * sizeof(decltype(vendor.data)::value_type)))}
    , _type{EFIBoot::File_path::MSG::Vendor::TYPE}
{
    data.detach();
    static_assert(sizeof(guid) == sizeof(vendor.guid));
    memcpy(reinterpret_cast<void *>(&guid), &vendor.guid, sizeof(vendor.guid));
}

FilePath::Vendor::Vendor(const EFIBoot::File_path::MEDIA::Vendor &vendor)
    : data{QByteArray::fromRawData(reinterpret_cast<const char *>(vendor.data.data()), static_cast<int>(vendor.data.size() * sizeof(decltype(vendor.data)::value_type)))}
    , _type{EFIBoot::File_path::MEDIA::Vendor::TYPE}
{
    data.detach();
    static_assert(sizeof(guid) == sizeof(vendor.guid));
    memcpy(reinterpret_cast<void *>(&guid), &vendor.guid, sizeof(vendor.guid));
}

auto FilePath::Vendor::toEFIBootFilePath() const -> EFIBoot::File_path::ANY
{
    switch(_type)
    {
    case EFIBoot::File_path::HW::Vendor::TYPE:
    {
        EFIBoot::File_path::HW::Vendor value = {};
        static_assert(sizeof(guid) == sizeof(value.guid));
        memcpy(value.guid.data(), &guid, sizeof(guid));
        value.data.resize(static_cast<size_t>(data.size()));
        std::copy(std::begin(data), std::end(data), std::begin(value.data));
        return value;
    }

    case EFIBoot::File_path::MSG::Vendor::TYPE:
    {
        EFIBoot::File_path::MSG::Vendor value = {};
        static_assert(sizeof(guid) == sizeof(value.guid));
        memcpy(value.guid.data(), &guid, sizeof(guid));
        value.data.resize(static_cast<size_t>(data.size()));
        std::copy(std::begin(data), std::end(data), std::begin(value.data));
        return value;
    }

    case EFIBoot::File_path::MEDIA::Vendor::TYPE:
    {
        EFIBoot::File_path::MEDIA::Vendor value = {};
        static_assert(sizeof(guid) == sizeof(value.guid));
        memcpy(value.guid.data(), &guid, sizeof(guid));
        value.data.resize(static_cast<size_t>(data.size()));
        std::copy(std::begin(data), std::end(data), std::begin(value.data));
        return value;
    }

    default:
        return {};
    }
}

auto FilePath::Vendor::fromJSON(const QJsonObject &obj) -> std::optional<Vendor>
{
    Vendor value{};
    check_obj();
    try_read_3(_type, Double, Int);
    check_type(guid, String);
    value.guid = QUuid::fromString(obj["guid"].toString());
    check_type(data, String);
    value.data = QByteArray::fromBase64(obj["data"].toString().toUtf8());
    return {value};
}

auto FilePath::Vendor::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["_type"] = _type;
    value["guid"] = guid.toString();
    value["data"] = static_cast<QString>(data.toBase64());
    return value;
}

auto FilePath::Vendor::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    const char *type_string = nullptr;
    switch(_type)
    {
    case EFIBoot::File_path::HW::Vendor::TYPE:
        type_string = "Hw";
        break;

    case EFIBoot::File_path::MSG::Vendor::TYPE:
        type_string = "Msg";
        break;

    case EFIBoot::File_path::MEDIA::Vendor::TYPE:
        type_string = "Media";
        break;

    default:
        type_string = "Unk";
        break;
    }

    return _string = QString("Ven%1(%2,[%3B])").arg(type_string, guid.toString(QUuid::WithoutBraces)).arg(data.size());
}

auto FilePath::End::fromJSON(const QJsonObject &obj) -> std::optional<End>
{
    End value{};
    check_obj();
    try_read_3(_subtype, Double, Int);
    return {value};
}

auto FilePath::End::toJSON() const -> QJsonObject
{
    QJsonObject end_instance;
    end_instance["type"] = TYPE;
    end_instance["subtype"] = SUBTYPE;
    end_instance["_subtype"] = _subtype;
    return end_instance;
}

auto FilePath::End::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    const char *subtype_string = "Unknown";
    switch(_subtype)
    {
    case EFIBoot::File_path::END::Instance::SUBTYPE:
        subtype_string = "Instance";
        break;

    case EFIBoot::File_path::END::Entire::SUBTYPE:
        subtype_string = "Entire";
        break;

    default:
        break;
    }

    return _string = QString("End(%1)").arg(subtype_string);
}

FilePath::Unknown::Unknown(const EFIBoot::File_path::Unknown &unknown)
    : data{QByteArray::fromRawData(reinterpret_cast<const char *>(unknown.data.data()), static_cast<int>(unknown.data.size() * sizeof(decltype(unknown.data)::value_type)))}
    , _type{unknown.TYPE}
    , _subtype{unknown.SUBTYPE}
{
    data.detach();
}

auto FilePath::Unknown::toEFIBootFilePath() const -> EFIBoot::File_path::Unknown
{
    EFIBoot::File_path::Unknown value = {};
    value.TYPE = _type;
    value.SUBTYPE = _subtype;
    value.data.resize(static_cast<size_t>(data.size()));
    std::copy(std::begin(data), std::end(data), std::begin(value.data));
    return value;
}

auto FilePath::Unknown::fromJSON(const QJsonObject &obj) -> std::optional<Unknown>
{
    Unknown value{};
    check_obj();
    try_read_3(_type, Double, Int);
    try_read_3(_subtype, Double, Int);
    check_type(data, String);
    value.data = QByteArray::fromBase64(obj["data"].toString().toUtf8());
    return {value};
}

auto FilePath::Unknown::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["_type"] = _type;
    value["_subtype"] = _subtype;
    value["data"] = static_cast<QString>(data.toBase64());
    return value;
}

auto FilePath::Unknown::toString(bool refresh) const -> QString
{
    if(_string.size() && !refresh)
        return _string;

    return _string = QString("Path(%1,%2,[%3B])").arg(toHex(_type), toHex(_subtype)).arg(data.size());
}

auto BootEntry::fromEFIBootLoadOption(
    const EFIBoot::Load_option &load_option) -> BootEntry
{
    BootEntry value{};
    value.description = QString::fromStdU16String(load_option.description);

    value.optional_data_format = OptionalDataFormat::Base64;
    if(toUnicode(value.optional_data, load_option.optional_data, "UTF-8") && !value.optional_data.contains(QChar(0)))
        value.optional_data_format = OptionalDataFormat::Utf8;

    if(value.optional_data_format == OptionalDataFormat::Base64 && load_option.optional_data.size() % sizeof(char16_t) == 0 && toUnicode(value.optional_data, load_option.optional_data, "UTF-16") && !value.optional_data.contains(QChar(0)))
        value.optional_data_format = OptionalDataFormat::Utf16;

    if(value.optional_data_format == OptionalDataFormat::Base64)
        value.optional_data = QByteArray::fromRawData(reinterpret_cast<const char *>(load_option.optional_data.data()), static_cast<int>(load_option.optional_data.size() * sizeof(decltype(load_option.optional_data)::value_type))).toBase64();

    value.attributes = load_option.attributes;

    for(const auto &file_path: load_option.device_path)
        value.device_path.push_back(std::visit([](const auto &path) -> FilePath::ANY
            { return path; },
            file_path));

    return value;
}

auto BootEntry::fromError(const QString &error) -> BootEntry
{
    BootEntry value{};
    value.is_error = true;
    value.description = "Error";
    value.error = error;
    return value;
}

auto BootEntry::toEFIBootLoadOption() const -> EFIBoot::Load_option
{
    if(is_error)
        return {};

    EFIBoot::Load_option load_option{};
    load_option.description = description.toStdU16String();
    {
        auto bytes = getRawOptionalData();
        auto begin = reinterpret_cast<const EFIBoot::Raw_data::value_type *>(bytes.constData());
        std::copy(begin, std::next(begin, bytes.size()), std::back_inserter(load_option.optional_data));
    }

    load_option.attributes = attributes;
    for(const auto &file_path: device_path)
        load_option.device_path.push_back(std::visit([](const auto &obj) -> EFIBoot::File_path::ANY
            { return obj.toEFIBootFilePath(); },
            file_path));

    return load_option;
}

auto BootEntry::fromJSON(const QJsonObject &obj) -> std::optional<BootEntry>
{
    BootEntry value{};
    try_read(description, String);
    try_read_3(optional_data_format, Double, Int);
    try_read(optional_data, String);
    try_read_3(attributes, Double, Int);
    try_read_3(efi_attributes, Double, Int);
    check_type(file_path, Array);
    const auto device_path = obj["file_path"].toArray();
    for(const auto file_path: device_path)
    {
        auto dp = file_path.toObject();
        auto path = get_default(FilePath::JSON_readers(), QString("%1/%2").arg(dp["type"].toString(), dp["subtype"].toString()), [](const auto &)
            { return std::nullopt; })(dp);
        if(!path)
            return std::nullopt;

        value.device_path.push_back(*path);
    }

    return {value};
}

auto BootEntry::toJSON() const -> QJsonObject
{
    if(is_error)
        return {};

    QJsonObject load_option;
    load_option["description"] = description;
    load_option["optional_data_format"] = static_cast<int>(optional_data_format);
    load_option["optional_data"] = optional_data;
    load_option["attributes"] = static_cast<int>(attributes);
    load_option["efi_attributes"] = static_cast<int>(efi_attributes);
    QJsonArray file_path_json;
    for(const auto &file_path: device_path)
        file_path_json.push_back(std::visit([](const auto &obj) -> QJsonObject
            { return obj.toJSON(); },
            file_path));

    load_option["file_path"] = file_path_json;
    return load_option;
}

auto BootEntry::formatDevicePath(bool refresh) const -> QString
{
    if(device_path.empty())
        return {};

    if(device_path_str.size() && !refresh)
        return device_path_str;

    device_path_str.clear();
    for(const auto &file_path: device_path)
    {
        if(!device_path_str.isEmpty())
            device_path_str += "/";

        device_path_str += std::visit([refresh](const auto &obj)
            { return obj.toString(refresh); },
            file_path);
    }

    return device_path_str;
}

QString BootEntry::getTitle() const
{
    return QString("%1 (%2)").arg(description, toHex(index, 4));
}

auto BootEntry::changeOptionalDataFormat(BootEntry::OptionalDataFormat format, bool test) -> bool
{
    if(format == optional_data_format)
        return true;

    auto bytes = getRawOptionalData();
    QString temp_optional_data;
    switch(format)
    {
    case OptionalDataFormat::Base64:
        temp_optional_data = bytes.toBase64();
        break;

    case OptionalDataFormat::Utf16:
        if(static_cast<uint>(bytes.size()) % sizeof(char16_t) != 0)
            return false;

        if(!toUnicode(temp_optional_data, bytes, "UTF-16"))
            return false;

        break;

    case OptionalDataFormat::Utf8:
        if(!toUnicode(temp_optional_data, bytes, "UTF-8"))
            return false;

        break;

    case OptionalDataFormat::Hex:
        temp_optional_data = bytes.toHex();
        break;
    }

    if(temp_optional_data.contains(QChar(0)))
        return false;

    if(!test)
    {
        optional_data_format = format;
        optional_data = temp_optional_data;
    }
    return true;
}

auto BootEntry::getRawOptionalData() const -> QByteArray
{
    QByteArray bytes;
    switch(optional_data_format)
    {
    case OptionalDataFormat::Base64:
        bytes = QByteArray::fromBase64(optional_data.toUtf8());
        break;

    case OptionalDataFormat::Utf16:
        bytes = fromUnicode(optional_data, "UTF-16");
        break;

    case OptionalDataFormat::Utf8:
        bytes = fromUnicode(optional_data, "UTF-8");
        break;

    case OptionalDataFormat::Hex:
        bytes = QByteArray::fromHex(optional_data.toUtf8());
        break;
    }

    return bytes;
}

#undef try_read_4
#undef try_read_3
#undef try_read
#undef check_type
#undef check_obj
