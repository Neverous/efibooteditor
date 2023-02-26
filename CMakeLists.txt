cmake_minimum_required(VERSION 3.16)

set(FULL_VERSION "$ENV{BUILD_VERSION}")
if(NOT ("${FULL_VERSION}" MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+"))
    set(FULL_VERSION "0.0.0-${FULL_VERSION}")
endif()

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" VERSION ${FULL_VERSION})
set(VERSION_MAJOR ${CMAKE_MATCH_1})
set(VERSION_MINOR ${CMAKE_MATCH_2})
set(VERSION_PATCH ${CMAKE_MATCH_3})

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
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_DEBUG_POSTFIX "d")

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME Program)
set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME_CAPITALIZED})
set(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION})
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

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
)

if(${QT_VERSION_MAJOR} GREATER 5)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core5Compat REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt::Core5Compat
    )
else()
    if(NOT COMMAND qt_add_translations)
        function(qt_add_translations TARGET)
            cmake_parse_arguments(arg "" "" "TS_FILES;INCLUDE_DIRECTORIES" ${ARGN})
            qt5_add_translation(QM_FILES ${arg_TS_FILES})

            get_target_property(BINARY_DIR ${TARGET} BINARY_DIR)
            set(QRC ${BINARY_DIR}/translations.qrc)
            file(WRITE ${QRC} "<RCC><qresource prefix=\"/i18n\">\n")
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

target_compile_definitions(${PROJECT_NAME} PRIVATE
    APPLICATION_NAME="${APPLICATION_NAME}"
    VERSION="${FULL_VERSION}"
    VERSION_MAJOR=${VERSION_MAJOR}
    VERSION_MINOR=${VERSION_MINOR}
    VERSION_PATCH=${VERSION_PATCH}
    PROJECT_NAME="${PROJECT_NAME}"
    PROJECT_DESCRIPTION="${PROJECT_DESCRIPTION}"
    PROJECT_HOMEPAGE_URL="${PROJECT_HOMEPAGE_URL}"
    APPLICATION_ICON="${APPLICATION_ICON}"
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0xFFFFFF
)

# GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wpedantic -Werror -pedantic -Wshadow -Wextra -Wconversion $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
    )
endif()

# Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC") # MSVC compatibility mode
        # Fix ignoring warnings in system includes
        set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX /imsvc)
        set(CMAKE_INCLUDE_SYSTEM_FLAG_C /imsvc)

        # Clang-cl for some reason ignores all normal methods...
        foreach(INCLUDE_DIR ${Qt${QT_VERSION_MAJOR}Core_INCLUDE_DIRS} ${Qt${QT_VERSION_MAJOR}Gui_INCLUDE_DIRS} ${Qt${QT_VERSION_MAJOR}Network_INCLUDE_DIRS} ${Qt${QT_VERSION_MAJOR}Widgets_INCLUDE_DIRS})
            target_compile_options(${PROJECT_NAME} PRIVATE "SHELL:/imsvc ${INCLUDE_DIR}")
        endforeach()

        # Disable compatibility warnings
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wno-c++20-compat
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-cast-qual
            -Wno-declaration-after-statement
            -Wno-deprecated-dynamic-exception-spec
            -Wno-exit-time-destructors
            -Wno-extra-semi-stmt
            -Wno-global-constructors
            -Wno-old-style-cast
            -Wno-redundant-parens
            -Wno-reserved-identifier
            -Wno-return-std-move-in-c++11
            -Wno-unknown-warning-option
            -Wno-zero-as-null-pointer-constant
        )
    else() # Standard
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wpedantic -Werror -pedantic -Wshadow -Wextra -Wconversion $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        )
    endif()
endif()

# MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Fix ignoring warnings in system includes
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX /external:I)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_C /external:I)

    target_compile_options(${PROJECT_NAME} PRIVATE
        # Ignore warnings in external includes
        /experimental:external
    )
endif()

if(WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:/std:c++latest>
        # Ignore warnings in external includes
        /external:anglebrackets /external:W0
        # Enable all warnings in application code
        /Wall /permissive- /WX
        # Disable some warnings
        # C4464: relative include path contains '..
        /wd4464
        # C4702: unreachable code
        /wd4702
        # C4710: 'function' : function not inlined
        /wd4710
        # C4711: function 'function' selected for inline expansion
        /wd4711
        # C4820: 'bytes' bytes padding added after construct 'member_name'
        /wd4820
        # C4866: compiler may not enforce left-to-right evaluation order for call to 'C++17 operator'
        /wd4866
        # C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
        /wd5045
        # C5262: implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases // supposedly fixed in VS 2022 17.5 Preview 2 - https://developercommunity.visualstudio.com/t/10163250#T-N10180326
        /wd5262
        # C5264: 'variable-name': 'const' variable is not used
        /wd5264
    )
endif()

# Sources:
target_sources(${PROJECT_NAME} PRIVATE
    src/bootentry.cpp
    src/bootentrydelegate.cpp
    src/bootentryform.cpp
    src/bootentrylistmodel.cpp
    src/bootentrylistview.cpp
    src/bootentrywidget.cpp
    src/devicepathdelegate.cpp
    src/devicepathdialog.cpp
    src/devicepathlistview.cpp
    src/devicepathproxylistmodel.cpp
    src/driveinfo.cpp
    src/efibooteditor.cpp
)

if(UNIX AND NOT APPLE)
    target_sources(${PROJECT_NAME} PRIVATE
        src/driveinfo.linux.cpp
        src/efivar-lite.linux.c
    )
endif()

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        src/driveinfo.win32.cpp
        src/efivar-lite.c
        src/efivar-lite.common.h
        src/efivar-lite.win32.c
        windows.rc
    )
endif()

if(APPLE)
    target_sources(${PROJECT_NAME} PRIVATE
        src/driveinfo.darwin.cpp
        src/efivar-lite.c
        src/efivar-lite.common.h
        src/efivar-lite.darwin.c
    )
endif()

# Headers:
target_sources(${PROJECT_NAME} PRIVATE
    include/bootentry.h
    include/bootentrydelegate.h
    include/bootentryform.h
    include/bootentrylistmodel.h
    include/bootentrylistview.h
    include/bootentrywidget.h
    include/compat.h
    include/devicepathdelegate.h
    include/devicepathdialog.h
    include/devicepathlistview.h
    include/devicepathproxylistmodel.h
    include/driveinfo.h
    include/efiboot.h
    include/efibooteditor.h
    include/efivar-lite/efiboot-loadopt.h
    include/efivar-lite/efivar-dp.h
    include/efivar-lite/efivar.h
    include/qlabelwrapped.h
    include/qwidgetitemdelegate.h
)

# Forms:
target_sources(${PROJECT_NAME} PRIVATE
    src/form/bootentryform.ui
    src/form/bootentrywidget.ui
    src/form/devicepathdialog.ui
    src/form/efibooteditor.ui
)

# Resources:
qt_add_resources(RESOURCES icons.qrc)
target_sources(${PROJECT_NAME} PRIVATE ${RESOURCES})

# Translations
FILE(GLOB TRANSLATIONS
    ${CMAKE_SOURCE_DIR}/translations/${PROJECT_NAME}_*.ts
)

qt_add_translations(${PROJECT_NAME}
    TS_FILES ${TRANSLATIONS}
    INCLUDE_DIRECTORIES "include")

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework CoreFoundation"
        "-framework DiskArbitration"
        "-framework IOKit"
    )

elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        efiboot
        efivar
    )

endif()

if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
        /MANIFESTUAC:level=\'requireAdministrator\'
    )
endif()

if(("${CMAKE_BUILD_TYPE}" STREQUAL "Release") AND UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME
        DESTINATION bin
    LIBRARY
        DESTINATION lib
    ARCHIVE
        DESTINATION lib
    BUNDLE
        DESTINATION dist
)

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT Runtime)
include(InstallRequiredSystemLibraries)

# Bundle Qt Libraries
if(WIN32)
    if(NOT WINDEPLOYQT_EXECUTABLE)
        find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS ${QT${QT_VERSION_MAJOR}_INSTALL_PREFIX}/bin REQUIRED)
    endif()

    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${WINDEPLOYQT_EXECUTABLE} --dir ${CMAKE_BINARY_DIR}/qt --no-compiler-runtime --pdb "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    )

    install(DIRECTORY "${CMAKE_BINARY_DIR}/qt/"
        TYPE BIN
        COMPONENT Runtime
        PATTERN "*.pdb" EXCLUDE
    )

    if(("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"))
        install(FILES "$<TARGET_PDB_FILE:${PROJECT_NAME}>"
            TYPE BIN
            COMPONENT Debug
        )

        install(DIRECTORY "${CMAKE_BINARY_DIR}/qt/"
            TYPE BIN
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
        find_program(LINUXDEPLOY_EXECUTABLE NAMES linuxdeploy linuxdeploy-x86_64.AppImage)
    endif()

    if(LINUXDEPLOY_EXECUTABLE)
        get_target_property(QMAKE_EXECUTABLE Qt::qmake IMPORTED_LOCATION)
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
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_ICON "${APPLICATION_ICON}")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(CPACK_STRIP_FILES TRUE)
endif()

if(WIN32)
    set(CPACK_PACKAGE_TARGET_OS_NAME "windows")
    set(CPACK_GENERATOR WIX ZIP)

    set(CPACK_WIX_LIGHT_EXTRA_FLAGS "-dcl:high")
    set(CPACK_WIX_PRODUCT_ICON ${CPACK_PACKAGE_ICON})
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/misc/wix_banner.png")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/misc/wix_dialog.png")
    set(CPACK_WIX_PROPERTY_ARPHELPLINK "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_WIX_ROOT_FEATURE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
    set(CPACK_WIX_UPGRADE_GUID "4DC9BB3B-A552-49D8-A04B-5C13353DB826")

elseif(APPLE)
    set(CPACK_PACKAGE_TARGET_OS_NAME "macosx")
    set(CPACK_GENERATOR DragNDrop TZST)

    set(CPACK_BUNDLE_ICON ${CPACK_PACKAGE_ICON})
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY NO)

else()
    include(GNUInstallDirs)
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

    set(CPACK_PACKAGE_TARGET_OS_NAME "linux")
    set(CPACK_GENERATOR DEB TZST)
    set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPackLinux.cmake")

    set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd")
    set(CPACK_DEBIAN_PACKAGE_SECTION "admin")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

    # Built from GitHub actions
    if(NOT ("$ENV{BUILD_OS}" STREQUAL ""))
        # Because installed with aqt Qt needs to be specified manually in dependencies
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt${QT_VERSION_MAJOR}gui${QT_VERSION_MAJOR} (>= ${Qt${QT_VERSION_MAJOR}_VERSION}), libqt${QT_VERSION_MAJOR}widgets${QT_VERSION_MAJOR} (>= ${Qt${QT_VERSION_MAJOR}_VERSION})")

        if(${QT_VERSION_MAJOR} GREATER 5)
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt${QT_VERSION_MAJOR}core${QT_VERSION_MAJOR} (>= ${Qt${QT_VERSION_MAJOR}_VERSION}), libqt${QT_VERSION_MAJOR}core5compat${QT_VERSION_MAJOR} (>= ${Qt${QT_VERSION_MAJOR}_VERSION}), ${CPACK_DEBIAN_PACKAGE_DEPENDS}")
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

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME_CAPITALIZED}-v${PROJECT_VERSION}-${CPACK_PACKAGE_TARGET_OS_NAME}-qt-${Qt${QT_VERSION_MAJOR}_VERSION}")

include(CPackComponent)
cpack_add_component(Program
    DISPLAY_NAME EFIBootEditor
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
