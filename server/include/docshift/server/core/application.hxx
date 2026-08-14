#ifndef DOCSHIFT_SERVER_CORE_APPLICATION_HXX
#define DOCSHIFT_SERVER_CORE_APPLICATION_HXX

#include <docshift/server/core/server_configuration.hxx>

namespace docshift::server::core {

/**
 * Owns the server startup sequence and process-level configuration.
 */
class Application {
public:
    explicit Application(const ServerConfiguration& configuration);

    /**
     * Starts the HTTP event loop and blocks until the process is stopped.
     */
    void run();

private:
    ServerConfiguration m_configuration;
};

} // namespace docshift::server::core

#endif
