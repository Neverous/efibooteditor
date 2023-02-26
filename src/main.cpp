// SPDX-License-Identifier: LGPL-3.0-or-later
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>

#include "efibooteditor.h"

auto main(int argc, char *argv[]) -> int
{
    QApplication a(argc, argv);
    a.setApplicationName(APPLICATION_NAME);
    QApplication::setApplicationVersion(VERSION);
    QIcon::setFallbackThemeName("Tango");
    if(QIcon::themeName().isEmpty())
        QIcon::setThemeName("Tango");

    if(auto error_message = EFIBoot::init(); error_message)
    {
        return QMessageBox::critical(
            nullptr,
            "EFI support required",
            QStringFromStdTString(*error_message));
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(a);

    EFIBootEditor w;
    w.show();

    QApplication::processEvents();
    w.resetBootConfiguration();
    return QApplication::exec();
}
