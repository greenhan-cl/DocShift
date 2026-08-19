#include <services/user_service.hxx>

#include <common/api_error.hxx>
#include <storage/in_memory_store.hxx>

namespace docshift {
namespace services {

models::UserProfile UserService::currentProfile() const {
    return storage::InMemoryStore::instance().currentUser();
}

models::UserProfile UserService::updateProfile(
    const std::string& username,
    const std::string& avatar_url,
    const bool has_avatar
) const {
    if (username.empty()) {
        throw common::ApiError(422, "USERNAME_INVALID", "昵称不能为空。");
    }
    if (username.size() > 32) {
        throw common::ApiError(422, "USERNAME_TOO_LONG", "昵称不能超过 32 个字符。");
    }
    if (has_avatar && avatar_url.size() > 1500000) {
        throw common::ApiError(422, "AVATAR_TOO_LARGE", "演示头像数据过大。");
    }

    return storage::InMemoryStore::instance().updateUser(username, avatar_url, has_avatar);
}

} // namespace services
} // namespace docshift
