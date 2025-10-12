cmake_minimum_required(VERSION 3.16)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)

add_executable(testefibootdata testefibootdata.cpp)

target_link_libraries(testefibootdata
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        ${PROJECT_NAME}-core
)

add_test(NAME EFIBootData COMMAND testefibootdata)
