add_library(docshift_server_core STATIC
    ${PROJECT_SOURCE_DIR}/include/docshift/server/core/application.hxx
    ${PROJECT_SOURCE_DIR}/include/docshift/server/core/server_configuration.hxx
    core/application.cc
    core/server_configuration.cc
)

add_library(docshift::server_core ALIAS docshift_server_core)

target_include_directories(docshift_server_core
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(docshift_server_core
    PUBLIC
        Drogon::Drogon
)

target_compile_features(docshift_server_core PUBLIC cxx_std_17)
docshift_enable_warnings(docshift_server_core)

add_library(docshift_server_http OBJECT
    ${PROJECT_SOURCE_DIR}/include/docshift/server/health/health_controller.hxx
    health/health_controller.cc
)

add_library(docshift::server_http ALIAS docshift_server_http)

target_include_directories(docshift_server_http
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(docshift_server_http
    PUBLIC
        Drogon::Drogon
)

target_compile_features(docshift_server_http PUBLIC cxx_std_17)
target_compile_definitions(docshift_server_http
    PRIVATE
        DOCSHIFT_SERVER_VERSION="${PROJECT_VERSION}"
)

docshift_enable_warnings(docshift_server_http)
