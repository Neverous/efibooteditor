// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QCommandLineParser>
#include <QCoreApplication>

#include "efibootdata.h"

class EFIBootEditorCLI: public QObject
{
    Q_OBJECT

    QCommandLineParser parser{};
    EFIBootData data{this};
    bool efi_supported;
    bool failed{false};

public:
    explicit EFIBootEditorCLI(const std::optional<tstring> &efi_error_message, QObject *parent = nullptr);
    EFIBootEditorCLI(const EFIBootEditorCLI &) = delete;
    EFIBootEditorCLI &operator=(const EFIBootEditorCLI &) = delete;
    ~EFIBootEditorCLI() override;

    bool process(const QCoreApplication &app);

public Q_SLOTS:
    void showError(const QString &message, const QString &details);
    void showProgress(size_t step, size_t total, const QString &details) const;
    void hideProgress() const;
};
