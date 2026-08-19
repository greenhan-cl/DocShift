#include <services/auth_service.hxx>

#include <common/api_error.hxx>
#include <storage/in_memory_store.hxx>

namespace docshift {
namespace services {

void AuthService::requestEmailVerification(const std::string& email) const {
    if (email.empty() || email.find('@') == std::string::npos) {
        throw common::ApiError(422, "EMAIL_INVALID", "请输入有效的邮箱地址。");
    }
}

models::UserProfile AuthService::login(
    const std::string& email,
    const std::string& verification_code
) const {
    requestEmailVerification(email);
    if (verification_code != "123456") {
        throw common::ApiError(401, "VERIFICATION_CODE_INVALID", "验证码不正确。");
    }

    return storage::InMemoryStore::instance().login(email);
}

} // namespace services
} // namespace docshift
