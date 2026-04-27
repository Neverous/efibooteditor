// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include <QApplication>
#include <QIcon>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTranslator>

#include "efibooteditor.h"
#include "efibooteditorcli.h"
#include "main.h"

auto main(int argc, char *argv[]) -> int
{
    // Check EFI support
    auto efi_error_message = EFIBoot::init();

    // Set CLI application first
    auto app = std::make_unique<QCoreApplication>(argc, argv);
    setupApplication();

    // Load translation
    auto translators = setupTranslations();

    // Run CLI if arguments were provided
    {
        EFIBootEditorCLI cli{efi_error_message};
        if(cli.process(*app))
        {
            QCoreApplication::processEvents();
            return 0;
        }
    }

    // Switch to GUI
    app.reset(); // need to destroy QCoreApplication first
    app = std::make_unique<QApplication>(argc, argv);
    // Need to reset the application configuration
    setupApplication();
    for(auto &translator: translators)
        QCoreApplication::installTranslator(&translator);

    // Setup GUI style
    setupStyle();

    // Show window and then force reload boot entries
    EFIBootEditor gui{efi_error_message};
    gui.show();
    if(!efi_error_message)
    {
        QApplication::processEvents();
        gui.reloadBootConfiguration();
    }

    return QCoreApplication::exec();
}
