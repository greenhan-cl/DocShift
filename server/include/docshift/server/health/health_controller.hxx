#ifndef DOCSHIFT_SERVER_HEALTH_HEALTH_CONTROLLER_HXX
#define DOCSHIFT_SERVER_HEALTH_HEALTH_CONTROLLER_HXX

#include <functional>

#include <drogon/HttpController.h>

namespace docshift::server::health {

/**
 * Exposes a dependency-free liveness endpoint for container health checks.
 */
class HealthController : public drogon::HttpController<HealthController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(HealthController::getHealth, "/api/v1/health", drogon::Get);
    METHOD_LIST_END

    void getHealth(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    ) const;
};

} // namespace docshift::server::health

#endif

