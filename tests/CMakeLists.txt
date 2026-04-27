cmake_minimum_required(VERSION 3.16)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

add_executable(testefibootdata testefibootdata.cpp)
qt_add_resources(TEST_ICONS ../icons.qrc)
add_executable(testefibooteditor testefibooteditor.cpp ${TEST_ICONS})

target_link_libraries(testefibootdata
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        ${PROJECT_NAME}-core
)

target_link_libraries(testefibooteditor
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        ${PROJECT_NAME}-core
)

add_test(NAME EFIBootData COMMAND testefibootdata)
add_test(NAME EFIBootEditor COMMAND testefibooteditor)
