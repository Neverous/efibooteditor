// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "efibooteditorcli.h"

#include <QTextStream>
#include <iostream>

EFIBootEditorCLI::EFIBootEditorCLI(const std::optional<tstring> &efi_error_message, QObject *parent)
    : QObject{parent}
    , efi_supported{!efi_error_message}
{
    parser.setApplicationDescription(tr("Boot Editor for (U)EFI based systems."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"e", "export"}, tr("Export configuration."), tr("FILE")});
    parser.addOption({{"d", "dump"}, tr("Dump raw EFI data."), tr("FILE")});
    parser.addOption({{"i", "import"}, tr("Import configuration from JSON (either from export or raw dump)."), tr("FILE")});
    parser.addOption({{"f", "force"}, tr("Force import, don't ask for confirmation.")});

    connect(&data, &EFIBootData::error, this, &EFIBootEditorCLI::showError);
    connect(&data, &EFIBootData::progress, this, &EFIBootEditorCLI::showProgress);
    connect(&data, &EFIBootData::done, this, &EFIBootEditorCLI::hideProgress);

    if(!efi_supported)
        showError(tr("EFI support required"), QStringFromStdTString(*efi_error_message));
}

EFIBootEditorCLI::~EFIBootEditorCLI()
{
}

bool EFIBootEditorCLI::process(const QCoreApplication &app)
{
    bool processed = false;
    parser.process(app);

    QTextStream ts{stdout};
    if(parser.isSet("export"))
    {
        if(!efi_supported)
            return true;

        processed = true;
        ts << tr("Loading EFI Boot Manager entries…") << Qt::endl;
        data.reload();
        ts << tr("Exporting boot configuration…") << Qt::endl;
        data.export_(parser.value("export"));
    }

    if(parser.isSet("dump"))
    {
        if(!efi_supported)
            return true;

        processed = true;
        ts << tr("Exporting boot configuration…") << Qt::endl;
        data.dump(parser.value("dump"));
    }

    if(parser.isSet("import"))
    {
        if(!efi_supported)
            return true;

        processed = true;
        ts << tr("Importing boot configuration…") << Qt::endl;
        data.import_(parser.value("import"));
        if(!failed)
        {
            ts << tr("Loaded %0 %1 entries").arg(data.boot_entries_list_model.rowCount()).arg(tr("Boot")) << Qt::endl;
            ts << tr("Loaded %0 %1 entries").arg(data.driver_entries_list_model.rowCount()).arg(tr("Driver")) << Qt::endl;
            ts << tr("Loaded %0 %1 entries").arg(data.sysprep_entries_list_model.rowCount()).arg(tr("System Preparation")) << Qt::endl;
            ts << tr("Loaded %0 %1 entries").arg(data.hot_keys_list_model.rowCount()).arg(tr("Hot Key")) << Qt::endl;
            bool save = true;
            if(!parser.isSet("force"))
            {
                ts << tr("Are you sure you want to save?\nYour EFI configuration will be overwritten!") << " [y/N]" << Qt::endl;
                std::string action;
                std::cin >> action;
                save = action == "y";
            }

            if(save)
            {
                ts << tr("Saving EFI Boot Manager entries…") << Qt::endl;
                data.save();
            }
        }
    }

    return processed;
}

void EFIBootEditorCLI::showError(const QString &message, const QString &details)
{
    QTextStream ts{stderr};
    ts << tr("ERROR: %0! %1").arg(message, details) << Qt::endl;
    failed = true;
}

void EFIBootEditorCLI::showProgress(size_t step, size_t total, const QString &details) const
{
    if(step >= total)
        total = step + 1;

    QTextStream ts{stdout};
    ts << QString("\r[%0%]\t(%1/%2)\t%3").arg(100 * step / total).arg(step).arg(total).arg(details);
}

void EFIBootEditorCLI::hideProgress() const
{
    QTextStream ts{stdout};
    ts << "\33[2K\r[100%]\t" << tr("Finished") << Qt::endl;
}
