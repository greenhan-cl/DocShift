#include <docshift/server/core/server_configuration.hxx>

#include <charconv>
#include <cstdlib>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>

namespace docshift::server::core {
namespace {

std::optional<std::string> readEnvironment(const char* name) {
    const char* _value = std::getenv(name);
    if (_value == nullptr || _value[0] == '\0') {
        return std::nullopt;
    }

    return std::string(_value);
}

std::uint64_t parseUnsigned(const std::string& value, const char* name) {
    std::uint64_t _parsed_value = 0;
    const char* _begin = value.data();
    const char* _end = value.data() + value.size();
    const auto _result = std::from_chars(_begin, _end, _parsed_value);

    if (_result.ec != std::errc() || _result.ptr != _end) {
        throw std::invalid_argument(std::string(name) + " must be an unsigned integer");
    }

    return _parsed_value;
}

std::uint16_t parsePort(const std::string& value) {
    const std::uint64_t _port = parseUnsigned(value, "DOCSHIFT_SERVER_PORT");
    if (_port == 0 || _port > 65535) {
        throw std::out_of_range("DOCSHIFT_SERVER_PORT must be between 1 and 65535");
    }

    return static_cast<std::uint16_t>(_port);
}

std::size_t parseThreadCount(const std::string& value) {
    const std::uint64_t _thread_count = parseUnsigned(value, "DOCSHIFT_SERVER_THREADS");
    if (_thread_count == 0) {
        throw std::out_of_range("DOCSHIFT_SERVER_THREADS must be greater than zero");
    }

    if (_thread_count > std::numeric_limits<std::size_t>::max()) {
        throw std::out_of_range("DOCSHIFT_SERVER_THREADS is too large");
    }

    return static_cast<std::size_t>(_thread_count);
}

std::size_t defaultThreadCount() {
    const unsigned int _hardware_threads = std::thread::hardware_concurrency();
    return _hardware_threads == 0 ? 1 : static_cast<std::size_t>(_hardware_threads);
}

} // namespace

ServerConfiguration::ServerConfiguration(
    const std::string& listen_address,
    const std::uint16_t listen_port,
    const std::size_t thread_count,
    const std::filesystem::path& data_root
)
    : m_listen_address(listen_address),
      m_listen_port(listen_port),
      m_thread_count(thread_count),
      m_data_root(data_root) {
}

ServerConfiguration ServerConfiguration::fromEnvironment() {
    std::string _listen_address = readEnvironment("DOCSHIFT_SERVER_ADDRESS").value_or("0.0.0.0");
    std::uint16_t _listen_port = 8080;
    std::size_t _thread_count = defaultThreadCount();
    std::filesystem::path _data_root = readEnvironment("DOCSHIFT_DATA_ROOT").value_or("./data");

    if (const auto _port_value = readEnvironment("DOCSHIFT_SERVER_PORT")) {
        _listen_port = parsePort(*_port_value);
    }

    if (const auto _thread_value = readEnvironment("DOCSHIFT_SERVER_THREADS")) {
        _thread_count = parseThreadCount(*_thread_value);
    }

    return ServerConfiguration(
        _listen_address,
        _listen_port,
        _thread_count,
        _data_root
    );
}

const std::string& ServerConfiguration::listenAddress() const noexcept {
    return m_listen_address;
}

std::uint16_t ServerConfiguration::listenPort() const noexcept {
    return m_listen_port;
}

std::size_t ServerConfiguration::threadCount() const noexcept {
    return m_thread_count;
}

const std::filesystem::path& ServerConfiguration::dataRoot() const noexcept {
    return m_data_root;
}

} // namespace docshift::server::core
