cmake_minimum_required(VERSION 3.16)

project(docshift_server
    VERSION 0.1.0
    DESCRIPTION "DocShift HTTP API service"
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Threads REQUIRED)

include(CompilerWarnings)

add_library(docshift_server_runtime STATIC
    include/common/api_error.hxx
    include/common/json_serialization.hxx
    include/config/server_configuration.hxx
    include/http/http_server.hxx
    include/models/conversion_item.hxx
    include/models/task_export.hxx
    include/models/task_summary.hxx
    include/models/user_profile.hxx
    include/services/auth_service.hxx
    include/services/conversion_service.hxx
    include/services/export_service.hxx
    include/services/user_service.hxx
    include/services/task_service.hxx
    include/storage/in_memory_store.hxx
    include/storage/local_file_storage.hxx
    src/common/api_error.cc
    src/common/json_serialization.cc
    src/config/server_configuration.cc
    src/http/http_server.cc
    src/services/auth_service.cc
    src/services/conversion_service.cc
    src/services/export_service.cc
    src/services/user_service.cc
    src/services/task_service.cc
    src/storage/in_memory_store.cc
    src/storage/local_file_storage.cc
)

add_library(docshift::runtime ALIAS docshift_server_runtime)

target_include_directories(docshift_server_runtime
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(docshift_server_runtime
    PUBLIC
        Threads::Threads
)

target_compile_features(docshift_server_runtime PUBLIC cxx_std_11)
target_compile_definitions(docshift_server_runtime
    PRIVATE
        DOCSHIFT_SERVER_VERSION="${PROJECT_VERSION}"
)
docshift_enable_warnings(docshift_server_runtime)

add_executable(docshift_server
    src/main.cc
)

target_link_libraries(docshift_server
    PRIVATE
        docshift::runtime
)

target_compile_definitions(docshift_server
    PRIVATE
        DOCSHIFT_SERVER_VERSION="${PROJECT_VERSION}"
)

target_compile_features(docshift_server PRIVATE cxx_std_11)
docshift_enable_warnings(docshift_server)

install(TARGETS docshift_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
