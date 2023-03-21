// SPDX-License-Identifier: LGPL-3.0-or-later
#include <QApplication>
#include <QCommandLineParser>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTranslator>

#include "efibooteditor.h"

auto main(int argc, char *argv[]) -> int
{
    QApplication a(argc, argv);
    a.setApplicationName(APPLICATION_NAME);
    QApplication::setApplicationVersion(VERSION);
    QIcon::setFallbackThemeName("Tango");
    if(QIcon::themeName().isEmpty())
        QIcon::setThemeName("Tango");

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const auto translations_path = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    const auto translations_path = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    // Enable translation
    for(const char *module: {"qt", "qtbase", PROJECT_NAME})
    {
        std::unique_ptr<QTranslator> translator{std::make_unique<QTranslator>()};
        if(!translator->load(QLocale::system(), module, "_", ":/i18n") && !translator->load(QLocale::system(), module, "_", translations_path))
            continue;

        a.installTranslator(translator.release());
    }

    // Check for EFI support
    if(auto error_message = EFIBoot::init(); error_message)
    {
        return QMessageBox::critical(
            nullptr,
            EFIBootEditor::tr("EFI support required"),
            QStringFromStdTString(*error_message));
    }

    // Command line support
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(a);

    // Show window
    EFIBootEditor w;
    w.show();

    QApplication::processEvents();
    w.reloadBootConfiguration();
    return QApplication::exec();
}
