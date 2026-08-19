#ifndef DOCSHIFT_COMMON_JSON_SERIALIZATION_HXX
#define DOCSHIFT_COMMON_JSON_SERIALIZATION_HXX

#include <string>

#include <nlohmann/json.hpp>

#include <models/conversion_item.hxx>
#include <models/task_export.hxx>
#include <models/task_summary.hxx>
#include <models/user_profile.hxx>

namespace docshift {
namespace common {

nlohmann::json toJson(const models::UserProfile& user);
nlohmann::json toJson(const models::ConversionItem& item);
nlohmann::json toJson(const models::Preview& preview);
nlohmann::json toJson(const models::TaskExport& task_export);
nlohmann::json toJson(const models::TaskSummary& task);
std::string toStatusText(models::ConversionStatus status);

} // namespace common
} // namespace docshift

#endif
