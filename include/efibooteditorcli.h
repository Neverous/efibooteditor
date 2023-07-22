// SPDX-License-Identifier: LGPL-3.0-or-later
#include <QCommandLineParser>
#include <QCoreApplication>

#include "compat.h"
#include "efibootdata.h"

class EFIBootEditorCLI: public QObject
{
    Q_OBJECT

    QCommandLineParser parser{};
    EFIBootData data{this};
    bool efi_supported;

public:
    explicit EFIBootEditorCLI(const std::optional<std::tstring> &efi_error_message, QObject *parent = nullptr);
    EFIBootEditorCLI(const EFIBootEditorCLI &) = delete;
    EFIBootEditorCLI &operator=(const EFIBootEditorCLI &) = delete;
    ~EFIBootEditorCLI() override;

    bool process(QCoreApplication &app);

public slots:
    void showError(const QString &message, const QString &details);
    void showProgress(size_t step, size_t total, const QString &details);
    void hideProgress();
};
