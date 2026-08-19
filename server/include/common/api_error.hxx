#ifndef DOCSHIFT_COMMON_API_ERROR_HXX
#define DOCSHIFT_COMMON_API_ERROR_HXX

#include <stdexcept>
#include <string>

namespace docshift {
namespace common {

class ApiError : public std::runtime_error {
public:
    ApiError(
        int status_code,
        const std::string& error_code,
        const std::string& message
    );

    int statusCode() const noexcept;
    const std::string& errorCode() const noexcept;

private:
    int m_status_code;
    std::string m_error_code;
};

} // namespace common
} // namespace docshift

#endif
