#include <services/task_service.hxx>

#include <common/api_error.hxx>
#include <storage/in_memory_store.hxx>

namespace docshift {
namespace services {

std::vector<models::TaskSummary> TaskService::list() const {
    return storage::InMemoryStore::instance().taskSummaries();
}

models::TaskSummary TaskService::create(const std::string& display_name) const {
    if (display_name.size() > 64) {
        throw common::ApiError(422, "TASK_NAME_TOO_LONG", "Task 名称不能超过 64 个字符。");
    }

    return storage::InMemoryStore::instance().createTask(
        display_name.empty() ? "未命名任务" : display_name
    );
}

models::TaskSummary TaskService::find(const std::string& task_id) const {
    models::TaskSummary _task;
    if (!storage::InMemoryStore::instance().findTask(task_id, &_task)) {
        throw common::ApiError(404, "TASK_NOT_FOUND", "Task 不存在。");
    }

    return _task;
}

std::vector<models::ConversionItem> TaskService::conversionItems(const std::string& task_id) const {
    find(task_id);
    return storage::InMemoryStore::instance().conversionItems(task_id);
}

} // namespace services
} // namespace docshift
