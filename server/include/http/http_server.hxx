#ifndef DOCSHIFT_HTTP_HTTP_SERVER_HXX
#define DOCSHIFT_HTTP_HTTP_SERVER_HXX

#include <config/server_configuration.hxx>

namespace docshift {
namespace http {

/**
 * Owns the HTTP server startup sequence and process-level configuration.
 */
class HttpServer {
public:
    explicit HttpServer(const config::ServerConfiguration& configuration);

    /**
     * Starts the HTTP event loop and blocks until the process is stopped.
     */
    void run();

private:
    config::ServerConfiguration m_configuration;
};

} // namespace http
} // namespace docshift

#endif
