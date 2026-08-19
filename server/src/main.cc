#include <cstdlib>
#include <exception>
#include <iostream>

#include <config/server_configuration.hxx>
#include <http/http_server.hxx>

int main() {
    try {
        auto _configuration = docshift::config::ServerConfiguration::fromEnvironment();
        docshift::http::HttpServer _server(_configuration);
        _server.run();
    } catch (const std::exception& _exception) {
        std::cerr << "Failed to start docshift-server: " << _exception.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
