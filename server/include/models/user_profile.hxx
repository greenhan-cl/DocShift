#ifndef DOCSHIFT_MODELS_USER_PROFILE_HXX
#define DOCSHIFT_MODELS_USER_PROFILE_HXX

#include <string>

namespace docshift {
namespace models {

struct UserProfile {
    std::string email;
    std::string username;
    std::string avatar_url;
    bool has_avatar;
};

} // namespace models
} // namespace docshift

#endif
