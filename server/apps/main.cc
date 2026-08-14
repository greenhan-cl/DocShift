#include <cstdlib>
#include <exception>
#include <iostream>

#include <docshift/server/core/application.hxx>
#include <docshift/server/core/server_configuration.hxx>

int main() {
    try {
        auto _configuration = docshift::server::core::ServerConfiguration::fromEnvironment();
        docshift::server::core::Application _application(_configuration);
        _application.run();
    } catch (const std::exception& _exception) {
        std::cerr << "Failed to start docshift-server: " << _exception.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
