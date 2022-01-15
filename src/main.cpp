// SPDX-License-Identifier: LGPL-3.0-or-later
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>

#include "efibooteditor.h"

auto main(int argc, char *argv[]) -> int
{
    QApplication a(argc, argv);
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

    EFIBootEditor w;
    w.show();

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(a);
    return QApplication::exec();
}
