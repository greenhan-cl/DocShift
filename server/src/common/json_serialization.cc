#include <common/json_serialization.hxx>

namespace docshift {
namespace common {

nlohmann::json toJson(const models::UserProfile& user) {
    return {{"email", user.email}, {"username", user.username}, {"avatar_url", user.has_avatar ? nlohmann::json(user.avatar_url) : nlohmann::json()}};
}

nlohmann::json toJson(const models::ConversionItem& item) {
    nlohmann::json _json;
    _json["item_id"] = item.item_id;
    _json["task_id"] = item.task_id;
    _json["source"] = {{"filename", item.source.filename}, {"format", item.source.format}, {"size_bytes", item.source.size_bytes}};
    _json["target_format"] = item.target_format;
    _json["status"] = toStatusText(item.status);
    _json["download_available"] = item.download_available;
    _json["error"] = item.has_error ? nlohmann::json{{"code", item.error.code}, {"message", item.error.message}} : nlohmann::json();
    return _json;
}

nlohmann::json toJson(const models::Preview& preview) { return {{"title", preview.title}, {"kind", preview.kind}, {"content", preview.content}}; }
nlohmann::json toJson(const models::TaskExport& task_export) { return {{"export_id", task_export.export_id}, {"filename", task_export.filename}, {"item_count", task_export.item_count}, {"download_url", task_export.download_url}}; }

nlohmann::json toJson(const models::TaskSummary& task) {
    return {
        {"task_id", task.task_id},
        {"display_name", task.display_name},
        {"counts", {
            {"total", task.counts.total},
            {"processing", task.counts.processing},
            {"preview_ready", task.counts.preview_ready},
            {"failed", task.counts.failed}
        }}
    };
}

std::string toStatusText(const models::ConversionStatus status) {
    switch (status) {
    case models::ConversionStatus::kQueued: return "queued";
    case models::ConversionStatus::kProcessing: return "processing";
    case models::ConversionStatus::kPreviewReady: return "preview_ready";
    case models::ConversionStatus::kFailed: return "failed";
    case models::ConversionStatus::kExpired: return "expired";
    case models::ConversionStatus::kDeleted: return "deleted";
    }
    return "failed";
}

} // namespace common
} // namespace docshift
