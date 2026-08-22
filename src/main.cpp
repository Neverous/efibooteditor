// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include <QApplication>
#include <QIcon>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTranslator>

#include "efibooteditor.h"
#ifndef Q_OS_WASM
#include "efibooteditorcli.h"
#endif
#include "main.h"

auto main(int argc, char *argv[]) -> int
{
    std::unique_ptr<QCoreApplication> app;

    // Check EFI support
    auto efi_error_message = EFIBoot::init();
#ifndef Q_OS_WASM
    // Set CLI application first
    app = std::make_unique<QCoreApplication>(argc, argv);
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

    app.reset(); // need to destroy QCoreApplication first
#endif

    // Switch to GUI
    app = std::make_unique<QApplication>(argc, argv);
    // Need to reset the application configuration
    setupApplication();
#ifndef Q_OS_WASM
    for(auto &translator: translators)
        QCoreApplication::installTranslator(&translator);
#else
    // Load translation
    setupTranslations();
#endif

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
