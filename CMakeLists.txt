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

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_DEBUG_POSTFIX "d")

# Link time optimization support check
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo)
if(ipo)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Default build type
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
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
        Widgets
    REQUIRED)

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
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

if(UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wpedantic -Werror -pedantic -Wshadow $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
    )
endif()

if(WIN32)
    # Fix ignoring warnings in system includes
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX /external:I)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_C /external:I)

    target_compile_options(${PROJECT_NAME} PRIVATE
        /std:c++latest /Wall /permissive- /WX
        # Ignore warnings in external includes
        /experimental:external /external:anglebrackets /external:W0
        # Disable some warnings
        # C4710: 'function' : function not inlined
        /wd4710
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
    )
endif()

if(UNIX)
    target_sources(${PROJECT_NAME} PRIVATE
        src/efivar-lite.unix.c
    )
endif()

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        src/driveinfo.win32.cpp
        src/efivar-lite.win32.c
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
    include/efivar-lite/efivar.h
    include/efivar-lite/efivar-dp.h
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

if(UNIX)
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

if(UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME
        DESTINATION bin
    LIBRARY
        DESTINATION lib
    ARCHIVE
        DESTINATION lib
)

include(InstallRequiredSystemLibraries)

install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION lib
)

if(TARGET Qt5::windeployqt)
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
        COMMAND set PATH=%PATH%$<SEMICOLON>${QT_INSTALL_PREFIX}/bin
        COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/dist" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    )

    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dist/"
        DESTINATION dist
    )
endif()
