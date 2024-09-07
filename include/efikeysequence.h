// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>
#include <tuple>

#include "efiboot.h"

static const QVector<std::tuple<int, QString, Qt::Key>> efi_scan_codes = {
    {0x00, "NULL", Qt::Key_unknown},
    {0x01, "Up", Qt::Key_Up},
    {0x02, "Down", Qt::Key_Down},
    {0x03, "Right", Qt::Key_Right},
    {0x04, "Left", Qt::Key_Left},
    {0x05, "Home", Qt::Key_Home},
    {0x06, "End", Qt::Key_End},
    {0x07, "Insert", Qt::Key_Insert},
    {0x08, "Delete", Qt::Key_Delete},
    {0x09, "Page Up", Qt::Key_PageUp},
    {0x0a, "Page Down", Qt::Key_PageDown},
    {0x0b, "F1", Qt::Key_F1},
    {0x0c, "F2", Qt::Key_F2},
    {0x0d, "F3", Qt::Key_F3},
    {0x0e, "F4", Qt::Key_F4},
    {0x0f, "F5", Qt::Key_F5},
    {0x10, "F6", Qt::Key_F6},
    {0x11, "F7", Qt::Key_F7},
    {0x12, "F8", Qt::Key_F8},
    {0x13, "F9", Qt::Key_F9},
    {0x14, "F10", Qt::Key_F10},
    {0x15, "F11", Qt::Key_F11},
    {0x16, "F12", Qt::Key_F12},
    {0x17, "ESC", Qt::Key_Escape},
    {0x48, "Pause", Qt::Key_Pause},
    {0x68, "F13", Qt::Key_F13},
    {0x69, "F14", Qt::Key_F14},
    {0x6a, "F15", Qt::Key_F15},
    {0x6b, "F16", Qt::Key_F16},
    {0x6c, "F17", Qt::Key_F17},
    {0x6d, "F18", Qt::Key_F18},
    {0x6e, "F19", Qt::Key_F19},
    {0x6f, "F20", Qt::Key_F20},
    {0x70, "F21", Qt::Key_F21},
    {0x71, "F22", Qt::Key_F22},
    {0x72, "F23", Qt::Key_F23},
    {0x73, "F24", Qt::Key_F24},
    {0x7f, "Mute", Qt::Key_VolumeMute},
    {0x80, "Volume Up", Qt::Key_VolumeUp},
    {0x81, "Volume Down", Qt::Key_VolumeDown},
    {0x100, "Brightness Up", Qt::Key_unknown}, // Qt::Key_BrightnessUp
    {0x101, "Brightness Down", Qt::Key_unknown}, // Qt::Key_BrightnessUp
    {0x102, "Suspend", Qt::Key_Suspend},
    {0x103, "Hibernate", Qt::Key_Hibernate},
    {0x104, "Toggle Display", Qt::Key_Display},
    {0x105, "Recovery", Qt::Key_unknown}, // Qt::Key_Recovery
    {0x106, "Eject", Qt::Key_Eject},
};

static const QVector<std::tuple<QString, Qt::Key>> efi_modifiers = {
    {"Shift", Qt::Key_Shift},
    {"Ctrl", Qt::Key_Control},
    {"Alt", Qt::Key_Alt},
    {"Meta", Qt::Key_Meta},
    {"Menu", Qt::Key_Menu},
    {"SysReq", Qt::Key_SysReq},
};

class EFIKey
{
private:
    Qt::Key scan_code{};
    QChar unicode_char{};

public:
    EFIKey() = default;
    explicit EFIKey(const EFIBoot::efi_input_key &key);
    explicit EFIKey(const Qt::Key _scan_code, const QChar _unicode_char);
    EFIBoot::efi_input_key toEFIInputKey() const;
    bool operator==(const EFIKey &b) const;
    bool operator!=(const EFIKey &b) const { return !(*this == b); }

    static EFIKey fromString(const QString &repr, bool *success = nullptr);
    QString toString() const;

    bool isUnicode() const { return scan_code == Qt::Key_unknown; }
    EFIKey toUpper() const;
    bool isUpper() const;

    static EFIKey fromQKey(int key, Qt::KeyboardModifiers modifiers, const QString &text, bool *success = nullptr);
};

class EFIKeySequence
{
    Q_GADGET

private:
    QSet<Qt::Key> shift_state{};
    QList<EFIKey> keys{};

public:
    EFIKeySequence() = default;
    EFIKeySequence(const EFIBoot::efi_boot_key_data &key_data, const std::vector<EFIBoot::efi_input_key> &keys_);

    bool toEFIKeyOption(EFIBoot::efi_boot_key_data &key_data, std::vector<EFIBoot::efi_input_key> &keys_) const;

    static EFIKeySequence fromString(const QString &str, qsizetype maxKeys);
    QString toString(bool escaped = false) const;

    bool isEmpty() const;

    bool operator==(const EFIKeySequence &b) const;
    bool operator!=(const EFIKeySequence &b) const { return !(*this == b); }

    bool addKey(int key, Qt::KeyboardModifiers modifiers, const QString &text, qsizetype maxKeys);

private:
    void fixShiftState();
};

Q_DECLARE_METATYPE(const EFIKeySequence *)
