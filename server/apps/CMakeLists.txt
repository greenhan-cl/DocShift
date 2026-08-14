add_executable(docshift_server
    main.cc
)

target_link_libraries(docshift_server
    PRIVATE
        docshift::server_core
        docshift::server_http
)

target_compile_features(docshift_server PRIVATE cxx_std_17)
docshift_enable_warnings(docshift_server)

install(TARGETS docshift_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
