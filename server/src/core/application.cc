#include <docshift/server/core/application.hxx>

#include <drogon/drogon.h>

namespace docshift::server::core {

Application::Application(const ServerConfiguration& configuration)
    : m_configuration(configuration) {
}

void Application::run() {
    auto& _application = drogon::app();
    _application.addListener(
        m_configuration.listenAddress(),
        m_configuration.listenPort()
    );
    _application.setThreadNum(m_configuration.threadCount());
    _application.run();
}

} // namespace docshift::server::core
