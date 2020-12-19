BUILD_VERSION = $$(BUILD_VERSION)
equals(BUILD_VERSION, "") {
    BUILD_VERSION = v0.0.0-local
}

VERSION = $$find(BUILD_VERSION, v.*\..*\..*-.*)
VERSION = $$split(VERSION, -)
VERSION = $$first(VERSION)
VERSION = $$split(VERSION, v)
VERSION = $$last(VERSION)
equals(VERSION, "") {
    VERSION = 0.0.0
}

QMAKE_TARGET_COMPANY        = "EFI Boot Editor"
QMAKE_TARGET_PRODUCT        = "EFI Boot Editor"
QMAKE_TARGET_DESCRIPTION    = "Boot Editor for (U)EFI based systems"
QMAKE_TARGET_COPYRIGaHT     = "Copyright &copy; 2020 EFI Boot Editor"

win32: QMAKE_TARGET_COPYRIGHT   = "Copyright (C) 2020 EFI Boot Editor"


QT += core gui widgets
greaterThan(QT_MAJOR_VERSION, 5): QT+= core5compat

CONFIG += debug_and_release c++latest rtti_off exceptions_off strict_c++ strict_c c11
win32: CONFIG += windows embed_manifest_exe windeployqt


DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x060000 VERSION=\\\"$${BUILD_VERSION}\\\"
win32: QMAKE_LFLAGS += /MANIFESTUAC:level=\'requireAdministrator\'

unix: QMAKE_CXXFLAGS += -Wall -Wpedantic -Werror -pedantic -Wshadow -Weffc++ -std=c++2a
unix: QMAKE_CFLAGS += -Wall -Wpedantic -Werror -pedantic -Wshadow -std=c11
win32: QMAKE_CXXFLAGS += /Wall /permissive- /WX /std:c++latest
win32: QMAKE_CFLAGS += /Wall /permissive- /WX

# Ignore warnings in qt includes
unix: QMAKE_CXXFLAGS += -isystem $$[QT_INSTALL_HEADERS]
win32: QMAKE_CXXFLAGS += /experimental:external /external:W0 /external:I $$[QT_INSTALL_HEADERS]

# C4355: 'this' : used in base member initializer list
win32: QMAKE_CXXFLAGS += /wd4355
# Disable some warnings from external libraries (QT, MSVC)
# C4371: 'classname': layout of class may have changed from a previous version of the compiler due to better packing of member 'member'
win32: QMAKE_CXXFLAGS += /wd4371
# C4619: #pragma warning : there is no warning number 'number'
win32: QMAKE_CXXFLAGS += /wd4619
# C4710: 'function' : function not inlined
win32: QMAKE_CXXFLAGS += /wd4710
win32: QMAKE_CFLAGS += /wd4710
# C4711: function 'function' selected for inline expansion
win32: QMAKE_CXXFLAGS += /wd4711
win32: QMAKE_CFLAGS += /wd4711
# C4946: reinterpret_cast used between related classes: 'class1' and 'class2'
win32: QMAKE_CXXFLAGS += /wd4946
# C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
win32: QMAKE_CXXFLAGS += /wd5045
win32: QMAKE_CFLAGS += /wd5045

# error: keyword is hidden by macro definition
unix: QMAKE_CXXFLAGS += -Wno-keyword-macro


SOURCES += \
    src/bootentry.cpp \
    src/bootentrydelegate.cpp \
    src/bootentryform.cpp \
    src/bootentrylistmodel.cpp \
    src/bootentrylistview.cpp \
    src/bootentrywidget.cpp \
    src/devicepathdelegate.cpp \
    src/devicepathdialog.cpp \
    src/devicepathlistview.cpp \
    src/devicepathproxylistmodel.cpp \
    src/driveinfo.cpp \
    src/efibooteditor.cpp \
    src/main.cpp

linux: SOURCES += \
    src/driveinfo.linux.cpp

unix: SOURCES += \
    src/efivar-lite.unix.c

win32: SOURCES += \
    src/efivar-lite.win32.c \
    src/driveinfo.win32.cpp

HEADERS += \
    src/include/bootentry.h \
    src/include/bootentrydelegate.h \
    src/include/bootentryform.h \
    src/include/bootentrylistmodel.h \
    src/include/bootentrylistview.h \
    src/include/bootentrywidget.h \
    src/include/compat.h \
    src/include/devicepathdelegate.h \
    src/include/devicepathdialog.h \
    src/include/devicepathlistview.h \
    src/include/devicepathproxylistmodel.h \
    src/include/driveinfo.h \
    src/include/efiboot.h \
    src/include/efibooteditor.h \
    src/include/efivar-lite/efivar-dp.h \
    src/include/qlabelwrapped.h \
    src/include/qwidgetitemdelegate.h \
    src/include/efivar-lite/efivar.h \
    src/include/efivar-lite/efiboot-loadopt.h

FORMS += \
    src/form/bootentryform.ui \
    src/form/bootentrywidget.ui \
    src/form/devicepathdialog.ui \
    src/form/efibooteditor.ui

unix: LIBS += -lefivar -lefiboot

DISTFILES += \
    .clang-format

RESOURCES += \
    icons.qrc

unix:CONFIG(release, debug|release): QMAKE_POST_LINK = $(STRIP) $(TARGET)
