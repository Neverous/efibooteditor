// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QCommandLineParser>
#include <QCoreApplication>

#include "compat.h"
#include "efibootdata.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QtGlobal>
namespace Qt
{
static const auto endl = ::endl;
}
#endif

class EFIBootEditorCLI: public QObject
{
    Q_OBJECT

    QCommandLineParser parser{};
    EFIBootData data{this};
    bool efi_supported;

public:
    explicit EFIBootEditorCLI(const std::optional<tstring> &efi_error_message, QObject *parent = nullptr);
    EFIBootEditorCLI(const EFIBootEditorCLI &) = delete;
    EFIBootEditorCLI &operator=(const EFIBootEditorCLI &) = delete;
    ~EFIBootEditorCLI() override;

    bool process(const QCoreApplication &app);

public slots:
    void showError(const QString &message, const QString &details);
    void showProgress(size_t step, size_t total, const QString &details);
    void hideProgress();
};
