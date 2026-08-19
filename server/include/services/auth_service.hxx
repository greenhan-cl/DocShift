#ifndef DOCSHIFT_SERVICES_AUTH_SERVICE_HXX
#define DOCSHIFT_SERVICES_AUTH_SERVICE_HXX

#include <string>

#include <models/user_profile.hxx>

namespace docshift {
namespace services {

class AuthService {
public:
    void requestEmailVerification(const std::string& email) const;
    models::UserProfile login(const std::string& email, const std::string& verification_code) const;
};

} // namespace services
} // namespace docshift

#endif
