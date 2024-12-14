// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTranslator>

#include "efibooteditor.h"
#include "efibooteditorcli.h"

auto main(int argc, char *argv[]) -> int
{
    // Check EFI support
    auto efi_error_message = EFIBoot::init();

    // Set CLI application first
    auto app = std::make_unique<QCoreApplication>(argc, argv);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(VERSION);
    QCoreApplication::setOrganizationName(APPLICATION_NAME);

    // Load translation
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const auto translations_path = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    const auto translations_path = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    std::list<QTranslator> translators;
    for(const char *module: {"qt", "qtbase", PROJECT_NAME})
    {
        auto &translator = translators.emplace_back();
        if(!translator.load(QLocale::system(), module, "_", "translations/")
            && !translator.load(QLocale::system(), module, "_", ":/translations/")
            && !translator.load(QLocale::system(), module, "_", translations_path))
        {
            translators.pop_back();
            continue;
        }

        QCoreApplication::installTranslator(&translator);
    }

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
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(VERSION);
    QCoreApplication::setOrganizationName(APPLICATION_NAME);
    for(auto &translator: translators)
        QCoreApplication::installTranslator(&translator);

    // Setup GUI style
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QIcon::setFallbackThemeName("Tango");
    if(QIcon::themeName().isEmpty())
        QIcon::setThemeName("Tango");

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
