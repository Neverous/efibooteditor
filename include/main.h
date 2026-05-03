// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QApplication>
#include <QIcon>
#include <QLibraryInfo>
#include <QStyleFactory>
#include <QTranslator>

inline void setupApplication()
{
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(VERSION);
    QCoreApplication::setOrganizationName(APPLICATION_NAME);
}

inline std::list<QTranslator> setupTranslations()
{
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

    return translators;
}

inline void setupStyle()
{
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << ":/icons");
    QIcon::setFallbackThemeName("Tango");
    if(!QIcon::hasThemeIcon("document-save"))
        QIcon::setThemeName("Tango");
}
