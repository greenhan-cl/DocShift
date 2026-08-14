cmake_minimum_required(VERSION 3.16)

project(docshift_server
    VERSION 0.1.0
    DESCRIPTION "DocShift HTTP API service"
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Drogon CONFIG REQUIRED)

include(CompilerWarnings)

add_subdirectory(src)
add_subdirectory(apps)

