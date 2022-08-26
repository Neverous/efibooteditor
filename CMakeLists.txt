cmake_minimum_required(VERSION 3.16)

set(VERSION "0.0.0")
if(NOT ("$ENV{BUILD_VERSION}" STREQUAL ""))
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" VERSION $ENV{BUILD_VERSION})
endif()

project(efibooteditor
    VERSION ${VERSION}
    DESCRIPTION "Boot Editor for (U)EFI based systems"
    LANGUAGES C CXX
)

set(PROJECT_NAME_CAPITALIZED "EFIBootEditor")

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
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0xFFFFFF
    VERSION="$ENV{BUILD_VERSION}"
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
        $<$<COMPILE_LANGUAGE:CXX>:/std:c++latest> /Wall /permissive- /WX
        # Ignore warnings in external includes
        /external:anglebrackets /external:W0
        # Disable some warnings
        # C4710: 'function' : function not inlined
        /wd4710
        # C4711: function 'function' selected for inline expansion
        /wd4711
        # C4866: compiler may not enforce left-to-right evaluation order for call to 'C++17 operator'
        /wd4866
        # C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
        /wd5045
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
    )

    if(("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") OR ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"))
        install(FILES "$<TARGET_PDB_FILE:${PROJECT_NAME}>"
            DESTINATION bin
            COMPONENT Debug
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
endif()

# CPack
set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME_CAPITALIZED})
set(CPACK_PACKAGE_CONTACT "Maciej Szeptuch <neverous@neverous.info>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_DIRECTORY dist)
set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} "${PROJECT_NAME_CAPITALIZED}")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Neverous/efibooteditor")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME_CAPITALIZED}")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME_CAPITALIZED}")
set(CPACK_PACKAGE_VENDOR ${PROJECT_NAME_CAPITALIZED})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/icons/Tango/scalable/categories/preferences-desktop.svg")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(CPACK_STRIP_FILES TRUE)
endif()

if(WIN32)
    set(CPACK_PACKAGE_TARGET_OS_NAME "windows")
    set(CPACK_GENERATOR WIX ZIP)

    set(CPACK_WIX_LIGHT_EXTRA_FLAGS "-dcl:high")
    set(CPACK_WIX_PRODUCT_ICON ${CPACK_PACKAGE_ICON})
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

    set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd")
    set(CPACK_DEBIAN_PACKAGE_SECTION "admin")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

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
