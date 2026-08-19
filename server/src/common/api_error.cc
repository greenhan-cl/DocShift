#include <common/api_error.hxx>

namespace docshift {
namespace common {

ApiError::ApiError(
    const int status_code,
    const std::string& error_code,
    const std::string& message
)
    : std::runtime_error(message),
      m_status_code(status_code),
      m_error_code(error_code) {
}

int ApiError::statusCode() const noexcept {
    return m_status_code;
}

const std::string& ApiError::errorCode() const noexcept {
    return m_error_code;
}

} // namespace common
} // namespace docshift
