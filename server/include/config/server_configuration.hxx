#ifndef DOCSHIFT_CONFIG_SERVER_CONFIGURATION_HXX
#define DOCSHIFT_CONFIG_SERVER_CONFIGURATION_HXX

#include <cstddef>
#include <cstdint>
#include <string>

namespace docshift {
namespace config {

/**
 * Stores the process-level settings required to start the HTTP service.
 */
class ServerConfiguration {
public:
    ServerConfiguration(
        const std::string& listen_address,
        std::uint16_t listen_port,
        std::size_t thread_count,
        const std::string& data_root,
        std::uint64_t upload_max_bytes
    );

    /**
     * Loads server settings from environment variables and validates them.
     */
    static ServerConfiguration fromEnvironment();

    const std::string& listenAddress() const noexcept;
    std::uint16_t listenPort() const noexcept;
    std::size_t threadCount() const noexcept;
    const std::string& dataRoot() const noexcept;
    std::uint64_t uploadMaxBytes() const noexcept;

private:
    std::string m_listen_address;
    std::uint16_t m_listen_port;
    std::size_t m_thread_count;
    std::string m_data_root;
    std::uint64_t m_upload_max_bytes;
};

} // namespace config
} // namespace docshift

#endif
