#ifndef DOCSHIFT_SERVICES_USER_SERVICE_HXX
#define DOCSHIFT_SERVICES_USER_SERVICE_HXX

#include <string>

#include <models/user_profile.hxx>

namespace docshift {
namespace services {

class UserService {
public:
    models::UserProfile currentProfile() const;
    models::UserProfile updateProfile(const std::string& username, const std::string& avatar_url, bool has_avatar) const;
};

} // namespace services
} // namespace docshift

#endif
