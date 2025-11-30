cmake_minimum_required(VERSION 3.16)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

string(REGEX REPLACE "^v" "" FULL_VERSION "$ENV{BUILD_VERSION}")
if(NOT ("${FULL_VERSION}" MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+"))
    set(FULL_VERSION "0.0.0-${FULL_VERSION}")
endif()

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" VERSION ${FULL_VERSION})
set(VERSION_MAJOR ${CMAKE_MATCH_1})
set(VERSION_MINOR ${CMAKE_MATCH_2})
set(VERSION_PATCH ${CMAKE_MATCH_3})
string(TIMESTAMP VERSION_TWEAK "%s" UTC)

message(STATUS "Building version ${VERSION} (${FULL_VERSION})")

project(efibooteditor
    VERSION ${VERSION}
    DESCRIPTION "Boot Editor for (U)EFI based systems."
    LANGUAGES C CXX
)

set(PROJECT_NAME_CAPITALIZED "EFIBootEditor")
set(PROJECT_HOMEPAGE_URL "https://github.com/Neverous/efibooteditor")
set(APPLICATION_NAME "EFI Boot Editor")
set(APPLICATION_ICON "${CMAKE_SOURCE_DIR}/icons/Tango/256/categories/preferences-system.ico")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS -b compat.h)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_DEBUG_POSTFIX "d")

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME Program)
set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME_CAPITALIZED})
set(MACOSX_BUNDLE_BUNDLE_VERSION ${FULL_VERSION})
set(MACOSX_BUNDLE_GUI_IDENTIFIER ${PROJECT_NAME_CAPITALIZED})


# Link time optimization support check
if(("${CMAKE_BUILD_TYPE}" STREQUAL "Release") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"))
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo)
    if(ipo)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if("${QT_VERSION_MAJOR}" STREQUAL "")
    find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
else()
    find_package(QT NAMES Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
endif()

message(STATUS "Qt: ${QT_VERSION_MAJOR}")

find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Network
        Widgets
        LinguistTools
    REQUIRED)

find_package(ZLIB REQUIRED)
message(STATUS "ZLIB: ${ZLIB_VERSION_STRING}")

add_library(${PROJECT_NAME}-core)

target_include_directories(${PROJECT_NAME}-core PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(${PROJECT_NAME}-core PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets
    ZLIB::ZLIB
)

if(${QT_VERSION_MAJOR} LESS_EQUAL 5)
    if(NOT COMMAND qt_add_translations)
        function(qt_add_translations TARGET)
            cmake_parse_arguments(arg "" "" "TS_FILES;RESOURCE_PREFIX;INCLUDE_DIRECTORIES;LUPDATE_OPTIONS" ${ARGN})
            qt5_add_translation(QM_FILES ${arg_TS_FILES})

            get_target_property(BINARY_DIR ${TARGET} BINARY_DIR)
            set(QRC ${BINARY_DIR}/translations.qrc)
            file(WRITE ${QRC} "<RCC><qresource prefix=\"${arg_RESOURCE_PREFIX}\">\n")
            foreach (QM_PATH ${QM_FILES})
                file(RELATIVE_PATH QM_FILE ${BINARY_DIR} ${QM_PATH})
                file(APPEND ${QRC} "<file>${QM_FILE}</file>\n")
            endforeach ()
            file(APPEND ${QRC} "</qresource></RCC>")
            qt_add_resources(RESOURCES ${QRC})
            target_sources(${PROJECT_NAME} PRIVATE ${RESOURCES})
        endfunction()
    endif()
endif()

add_compile_definitions(
    APPLICATION_NAME="${APPLICATION_NAME}"
    VERSION="${FULL_VERSION}"
    VERSION_MAJOR=${VERSION_MAJOR}
    VERSION_MINOR=${VERSION_MINOR}
    VERSION_PATCH=${VERSION_PATCH}
    VERSION_TWEAK=${VERSION_TWEAK}
    PROJECT_NAME="${PROJECT_NAME}"
    PROJECT_DESCRIPTION="${PROJECT_DESCRIPTION}"
    PROJECT_HOMEPAGE_URL="${PROJECT_HOMEPAGE_URL}"
    APPLICATION_ICON="${APPLICATION_ICON}"
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0xFFFFFF
)

# Sources:
target_sources(${PROJECT_NAME}-core PRIVATE
    src/bootentry.cpp
    src/bootentrydelegate.cpp
    src/bootentryform.cpp
    src/bootentrylistmodel.cpp
    src/bootentrylistview.cpp
    src/bootentrywidget.cpp
    src/commands.cpp
    src/devicepathproxymodel.cpp
    src/devicepathview.cpp
    src/driveinfo.cpp
    src/efibootdata.cpp
    src/efibooteditor.cpp
    src/efibooteditorcli.cpp
    src/efikeysequence.cpp
    src/efikeysequenceedit.cpp
    src/filepathdelegate.cpp
    src/filepathdialog.cpp
    src/hotkey.cpp
    src/hotkeydelegate.cpp
    src/hotkeylistmodel.cpp
    src/hotkeysdialog.cpp
    src/hotkeysview.cpp
)

if(UNIX AND NOT APPLE)
    target_sources(${PROJECT_NAME}-core PRIVATE
        src/driveinfo.linux.cpp
        src/efivar-lite.linux.c
    )
endif()

if(WIN32)
    target_sources(${PROJECT_NAME}-core PRIVATE
        src/driveinfo.win32.cpp
        src/efivar-lite.c
        src/efivar-lite.common.h
        src/efivar-lite.win32.c
    )
endif()

if(APPLE)
    target_sources(${PROJECT_NAME}-core PRIVATE
        src/driveinfo.darwin.cpp
        src/efivar-lite.c
        src/efivar-lite.common.h
        src/efivar-lite.darwin.c
    )
endif()

# Headers:
target_sources(${PROJECT_NAME}-core PRIVATE
    include/bootentry.h
    include/bootentrydelegate.h
    include/bootentryform.h
    include/bootentrylistmodel.h
    include/bootentrylistview.h
    include/bootentrywidget.h
    include/commands.h
    include/compat.h
    include/devicepathproxymodel.h
    include/devicepathview.h
    include/disableundoredo.h
    include/driveinfo.h
    include/efiboot.h
    include/efibootdata.h
    include/efibooteditor.h
    include/efibooteditorcli.h
    include/efikeysequence.h
    include/efikeysequenceedit.h
    include/efivar-lite/device-paths.h
    include/efivar-lite/efivar-lite.h
    include/efivar-lite/key-option.h
    include/efivar-lite/load-option.h
    include/filepathdelegate.h
    include/filepathdialog.h
    include/hotkey.h
    include/hotkeydelegate.h
    include/hotkeylistmodel.h
    include/hotkeysdialog.h
    include/hotkeysview.h
    include/qindicatorwidget.h
    include/qlabelwrapped.h
    include/qresizabletabwidget.h
    include/qwidgetitemdelegate.h
)

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
    src/main.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-core)

# Compile options
get_target_property(SOURCES ${PROJECT_NAME}-core SOURCES)

## GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Enable all warnings only in application code
    set_source_files_properties(${SOURCES} PROPERTIES COMPILE_OPTIONS "-Wall;-Wpedantic;-Werror;-pedantic;-Wshadow;-Wextra;-Wconversion;$<$<COMPILE_LANGUAGE:CXX>:-Weffc++>")
endif()

## Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC") # MSVC compatibility mode
        # Fix ignoring warnings in system includes
        set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX /imsvc)
        set(CMAKE_INCLUDE_SYSTEM_FLAG_C /imsvc)
    else() # Standard
        # Enable all warnings only in application code
        set_source_files_properties(${SOURCES} PROPERTIES COMPILE_OPTIONS "-Weverything;-pedantic;-Werror")
    endif()

    # Disable some compatibility warnings
    target_compile_options(${PROJECT_NAME}-core PUBLIC
        -Wno-c++20-compat
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-cast-qual
        -Wno-declaration-after-statement
        -Wno-exit-time-destructors
        -Wno-global-constructors
        -Wno-padded
        -Wno-return-std-move-in-c++11
        -Wno-switch-default
        -Wno-unknown-warning-option
        -Wno-unsafe-buffer-usage
        -Wno-poison-system-directories
    )
endif()

## MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Fix ignoring warnings in system includes
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX /external:I)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_C /external:I)

    target_compile_options(${PROJECT_NAME}-core PUBLIC
        /Z7
        # Ignore warnings in external includes
        /experimental:external /external:anglebrackets
    )
endif()

if(WIN32)
    target_compile_options(${PROJECT_NAME}-core PUBLIC
        # Ignore warnings in external includes
        /external:W0
    )

    # Enable all warnings only in application code
    set_source_files_properties(${SOURCES} PROPERTIES COMPILE_OPTIONS "/Wall;/permissive-;/WX")

    # Disable some warnings
    target_compile_options(${PROJECT_NAME}-core PUBLIC
        # C4371: 'classname': layout of class may have changed from a previous version of the compiler due to better packing of member 'member'
        /wd4371
        # C4710: 'function' : function not inlined
        /wd4710 # Needed for RELEASE builds
        # C4711: function 'function' selected for inline expansion
        /wd4711 # Needed for RELEASE builds
        # C4820: 'bytes' bytes padding added after construct 'member_name'
        /wd4820
        # C4866: compiler may not enforce left-to-right evaluation order for call to 'C++17 operator'
        /wd4866
        # C4868: compiler may not enforce left-to-right evaluation order in braced initializer list
        /wd4868
        # C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
        /wd5045
    )
endif()

# Forms:
target_sources(${PROJECT_NAME}-core PRIVATE
    src/form/bootentryform.ui
    src/form/bootentrywidget.ui
    src/form/efibooteditor.ui
    src/form/filepathdialog.ui
    src/form/hotkeysdialog.ui
)

# Resources:
qt_add_resources(RESOURCES icons.qrc)
target_sources(${PROJECT_NAME} PRIVATE ${RESOURCES})

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        windows.rc
    )
endif()

# Translations
FILE(GLOB TRANSLATIONS
    ${CMAKE_SOURCE_DIR}/translations/${PROJECT_NAME}_*.ts
)

set_source_files_properties(${TRANSLATIONS}
    PROPERTIES OUTPUT_LOCATION "${CMAKE_BINARY_DIR}/translations"
)

qt_add_translations(${PROJECT_NAME}
    TS_FILES ${TRANSLATIONS}
    RESOURCE_PREFIX "/"
    INCLUDE_DIRECTORIES "include"
    LUPDATE_OPTIONS "-no-obsolete")

# Libraries
if(APPLE)
    target_link_libraries(${PROJECT_NAME}-core PUBLIC
        "-framework CoreFoundation"
        "-framework DiskArbitration"
        "-framework IOKit"
    )

elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(EFIVAR REQUIRED efivar efiboot)

    target_link_libraries(${PROJECT_NAME}-core PUBLIC ${EFIVAR_LIBRARIES})
endif()

if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
        /MANIFESTUAC:level=\'requireAdministrator\'
    )
endif()

if(("${CMAKE_BUILD_TYPE}" STREQUAL "Release") AND UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

# Testing
enable_testing()
add_subdirectory(tests)

# Packaging
if(WIN32)
    install(TARGETS ${PROJECT_NAME}
        RUNTIME
            DESTINATION .
        LIBRARY
            DESTINATION .
        ARCHIVE
            DESTINATION .
        BUNDLE
            DESTINATION dist
    )

    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        install(FILES ${ZLIB_LIBRARY_DLL_DEBUG}
            DESTINATION .
            COMPONENT Runtime
        )
    else()
        install(FILES ${ZLIB_LIBRARY_DLL_RELEASE}
            DESTINATION .
            COMPONENT Runtime
        )
    endif()

    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
else()
    include(GNUInstallDirs)

    install(TARGETS ${PROJECT_NAME}
        RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
        BUNDLE
            DESTINATION dist
    )

    if(UNIX AND NOT APPLE)
        install(FILES misc/EFIBootEditor.desktop
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
            COMPONENT Desktop
        )
        install(FILES misc/org.x.efibooteditor.policy
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/polkit-1/actions/
            COMPONENT Desktop
        )
        install(FILES misc/EFIBootEditor.metainfo.xml
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
            COMPONENT Desktop
        )
    endif()
endif()

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT Runtime)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY TRUE)
endif()

include(InstallRequiredSystemLibraries)

# Bundle Qt Libraries
if(WIN32)
    if(NOT WINDEPLOYQT_EXECUTABLE)
        find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS ${QT${QT_VERSION_MAJOR}_INSTALL_PREFIX}/bin REQUIRED)
    endif()

    # windeployqt in 6.5.0 has broken translations support https://codereview.qt-project.org/c/qt/qtbase/+/468903
    if(${QT_VERSION} VERSION_EQUAL 6.5.0)
        set(SKIP_TRANSLATIONS "--no-translations")
    endif()

    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${WINDEPLOYQT_EXECUTABLE} --dir ${CMAKE_BINARY_DIR}/qt --no-compiler-runtime --pdb "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>" ${SKIP_TRANSLATIONS}
    )

    install(DIRECTORY "${CMAKE_BINARY_DIR}/qt/"
        DESTINATION .
        COMPONENT Runtime
        PATTERN "*.pdb" EXCLUDE
    )

    if(("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"))
        install(FILES "$<TARGET_PDB_FILE:${PROJECT_NAME}>"
            DESTINATION .
            COMPONENT Debug
        )

        install(DIRECTORY "${CMAKE_BINARY_DIR}/qt/"
            DESTINATION .
            COMPONENT Debug
            FILES_MATCHING
            PATTERN "*.pdb"
        )
    endif()

elseif(APPLE)
    if(NOT MACDEPLOYQT_EXECUTABLE)
        find_program(MACDEPLOYQT_EXECUTABLE NAMES macdeployqt HINTS ${QT${QT_VERSION_MAJOR}_INSTALL_PREFIX}/bin REQUIRED)
    endif()

    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${MACDEPLOYQT_EXECUTABLE} "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>" -always-overwrite
    )
else()
    if(NOT LINUXDEPLOY_EXECUTABLE)
        find_program(LINUXDEPLOY_EXECUTABLE NAMES linuxdeploy linuxdeploy.AppImage linuxdeploy-x86_64.AppImage linuxdeploy-aarch64.AppImage)
    endif()

    if(LINUXDEPLOY_EXECUTABLE)
        get_target_property(QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND QMAKE=${QMAKE_EXECUTABLE} ${LINUXDEPLOY_EXECUTABLE} --plugin=qt --appdir=${CMAKE_BINARY_DIR}/appdir/ --executable="$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        )

        install(DIRECTORY "${CMAKE_BINARY_DIR}/appdir/usr/"
            DESTINATION .
            COMPONENT Runtime
        )
    endif()
endif()

# CPack
set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME_CAPITALIZED})
set(CPACK_PACKAGE_CONTACT "Maciej Szeptuch <neverous@neverous.info>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_DIRECTORY dist)
set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} "${PROJECT_NAME_CAPITALIZED}")
set(CPACK_PACKAGE_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME_CAPITALIZED}")
set(CPACK_PACKAGE_NAME "${APPLICATION_NAME}")
set(CPACK_PACKAGE_VENDOR ${PROJECT_NAME_CAPITALIZED})
set(CPACK_PACKAGE_VERSION ${FULL_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_ICON "${APPLICATION_ICON}")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(CPACK_STRIP_FILES TRUE)
endif()

if(WIN32)
    set(CPACK_PACKAGE_TARGET_OS_NAME "windows")
    set(CPACK_GENERATOR WIX ZIP)
    set(CPACK_PACKAGE_VERSION ${VERSION}.${VERSION_TWEAK}) # WiX doesn't support SemVer

    set(CPACK_WIX_LIGHT_EXTRA_FLAGS "-dcl:high")
    set(CPACK_WIX_PRODUCT_ICON ${CPACK_PACKAGE_ICON})
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/misc/wix_banner.png")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/misc/wix_dialog.png")
    set(CPACK_WIX_PROPERTY_ARPHELPLINK "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_WIX_ROOT_FEATURE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
    set(CPACK_WIX_UPGRADE_GUID "4DC9BB3B-A552-49D8-A04B-5C13353DB826")
    set(CPACK_WIX_TEMPLATE "${CMAKE_SOURCE_DIR}/misc/WIX.template.in")

elseif(APPLE)
    set(CPACK_PACKAGE_TARGET_OS_NAME "macosx")
    set(CPACK_GENERATOR DragNDrop TZST)

    set(CPACK_BUNDLE_ICON ${CPACK_PACKAGE_ICON})
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY NO)
    # Seems like since macOS 13 hdiutil needs elevated access to work properly
    # at least in GitHub Actions https://github.com/actions/runner-images/issues/7522#issuecomment-1564467252
    set(CPACK_COMMAND_HDIUTIL "/usr/bin/sudo /usr/bin/hdiutil")

else()
    set(CPACK_PACKAGE_TARGET_OS_NAME "linux")
    set(CPACK_GENERATOR DEB TZST)
    set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_SOURCE_DIR}/cmake/CPackLinux.cmake")

    set(CPACK_DEBIAN_PACKAGE_NAME "${PROJECT_NAME_CAPITALIZED}")
    #set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd") Not supported on current Debian Bullseye
    set(CPACK_DEBIAN_PACKAGE_SECTION "admin")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

    # Built from GitHub actions
    if(NOT ("$ENV{BUILD_OS}" STREQUAL ""))
        # Because installed with aqt Qt needs to be specified manually in dependencies
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt${QT_VERSION_MAJOR}gui${QT_VERSION_MAJOR} (>= ${Qt${QT_VERSION_MAJOR}_VERSION}), libqt${QT_VERSION_MAJOR}widgets${QT_VERSION_MAJOR} (>= ${Qt${QT_VERSION_MAJOR}_VERSION}), libqt${QT_VERSION_MAJOR}network${QT_VERSION_MAJOR} (>= ${Qt${QT_VERSION_MAJOR}_VERSION})")

        if(${QT_VERSION_MAJOR} GREATER 5)
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt${QT_VERSION_MAJOR}core${QT_VERSION_MAJOR} (>= ${Qt${QT_VERSION_MAJOR}_VERSION}), ${CPACK_DEBIAN_PACKAGE_DEPENDS}")
        else()
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt${QT_VERSION_MAJOR}core${QT_VERSION_MAJOR}a (>= ${Qt${QT_VERSION_MAJOR}_VERSION}), ${CPACK_DEBIAN_PACKAGE_DEPENDS}")
        endif()
    endif()

    if(("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"))
        set(CPACK_DEBIAN_DEBUGINFO_PACKAGE ON)
    endif()
endif()

# Built from GitHub actions
if(NOT ("$ENV{BUILD_OS}" STREQUAL ""))
    set(CPACK_PACKAGE_TARGET_OS_NAME "$ENV{BUILD_OS}")
endif()

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_CAPITALIZED}-v${FULL_VERSION}-${CPACK_PACKAGE_TARGET_OS_NAME}-qt-${Qt${QT_VERSION_MAJOR}_VERSION}")

include(CPackComponent)
cpack_add_component(Program
    DISPLAY_NAME "EFI Boot Editor"
    DESCRIPTION "Main executable"
    REQUIRED
)

cpack_add_component(Desktop
    DISPLAY_NAME "Desktop files"
    DESCRIPTION "Useful files for running in Desktop Environment"
    DEPENDS Program
)

cpack_add_component(Runtime
    DISPLAY_NAME "Runtime libraries"
    DESCRIPTION "Necessary runtime libraries"
    DEPENDS Program
)

cpack_add_component(Debug
    DISPLAY_NAME "Debug symbols"
    DESCRIPTION "Debug symbols to aid troubleshooting"
    DISABLED
    DEPENDS Program
)

include(CPack)
