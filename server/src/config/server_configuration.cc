#include <config/server_configuration.hxx>

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>

namespace docshift {
namespace config {
namespace {

bool readEnvironment(const char* name, std::string* value) {
    const char* _value = std::getenv(name);
    if (_value == nullptr || _value[0] == '\0') {
        return false;
    }

    *value = _value;
    return true;
}

std::uint64_t parseUnsigned(const std::string& value, const char* name) {
    errno = 0;
    char* _end = nullptr;
    const unsigned long long _parsed_value = std::strtoull(value.c_str(), &_end, 10);
    if (errno == ERANGE || _end == value.c_str() || *_end != '\0') {
        throw std::invalid_argument(std::string(name) + " must be an unsigned integer");
    }

    return static_cast<std::uint64_t>(_parsed_value);
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
    const std::string& data_root,
    const std::uint64_t upload_max_bytes
)
    : m_listen_address(listen_address),
      m_listen_port(listen_port),
      m_thread_count(thread_count),
      m_data_root(data_root),
      m_upload_max_bytes(upload_max_bytes) {
}

ServerConfiguration ServerConfiguration::fromEnvironment() {
    std::string _listen_address = "0.0.0.0";
    std::uint16_t _listen_port = 8080;
    std::size_t _thread_count = defaultThreadCount();
    std::string _data_root = "./data";
    std::uint64_t _upload_max_bytes = 50ULL * 1024ULL * 1024ULL;
    std::string _environment_value;

    if (readEnvironment("DOCSHIFT_SERVER_ADDRESS", &_environment_value)) {
        _listen_address = _environment_value;
    }
    if (readEnvironment("DOCSHIFT_DATA_ROOT", &_environment_value)) {
        _data_root = _environment_value;
    }
    if (readEnvironment("DOCSHIFT_SERVER_PORT", &_environment_value)) {
        _listen_port = parsePort(_environment_value);
    }
    if (readEnvironment("DOCSHIFT_SERVER_THREADS", &_environment_value)) {
        _thread_count = parseThreadCount(_environment_value);
    }
    if (readEnvironment("DOCSHIFT_UPLOAD_MAX_BYTES", &_environment_value)) {
        _upload_max_bytes = parseUnsigned(_environment_value, "DOCSHIFT_UPLOAD_MAX_BYTES");
        if (_upload_max_bytes == 0) {
            throw std::out_of_range("DOCSHIFT_UPLOAD_MAX_BYTES must be greater than zero");
        }
    }
    const std::uint64_t _multipart_overhead_bytes = 1024ULL * 1024ULL;
    const std::uint64_t _max_payload_bytes = static_cast<std::uint64_t>(
        std::numeric_limits<std::size_t>::max()
    );
    if (_upload_max_bytes > _max_payload_bytes - _multipart_overhead_bytes) {
        throw std::out_of_range("DOCSHIFT_UPLOAD_MAX_BYTES is too large");
    }

    return ServerConfiguration(
        _listen_address,
        _listen_port,
        _thread_count,
        _data_root,
        _upload_max_bytes
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

const std::string& ServerConfiguration::dataRoot() const noexcept {
    return m_data_root;
}

std::uint64_t ServerConfiguration::uploadMaxBytes() const noexcept {
    return m_upload_max_bytes;
}

} // namespace config
} // namespace docshift
