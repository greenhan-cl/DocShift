#include <docshift/server/health/health_controller.hxx>

#include <utility>

#include <json/json.h>

namespace docshift::server::health {

void HealthController::getHealth(
    const drogon::HttpRequestPtr& request,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback
) const {
    static_cast<void>(request);

    Json::Value _body;
    _body["service"] = "docshift-server";
    _body["status"] = "ok";
    _body["version"] = DOCSHIFT_SERVER_VERSION;

    auto _response = drogon::HttpResponse::newHttpJsonResponse(_body);
    _response->setStatusCode(drogon::k200OK);
    callback(std::move(_response));
}

} // namespace docshift::server::health

