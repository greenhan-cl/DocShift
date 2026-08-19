#include <storage/in_memory_store.hxx>

#include <algorithm>
#include <cctype>
#include <utility>

#include <common/api_error.hxx>

namespace docshift {
namespace storage {
namespace {

constexpr char kDemoTaskId[] = "9a1a4c51-65f4-4a04-b81f-a6b2d84d9810";

models::ConversionItem makeDemoItem(
    const std::string& item_id,
    const std::string& filename,
    const std::string& format,
    const std::uint64_t size_bytes,
    const models::ConversionStatus status
) {
    models::ConversionItem _item;
    _item.item_id = item_id;
    _item.task_id = kDemoTaskId;
    _item.source = {filename, format, size_bytes, ""};
    _item.target_format = "markdown";
    _item.status = status;
    _item.download_available = status == models::ConversionStatus::kPreviewReady;
    _item.has_error = false;
    return _item;
}

} // namespace

InMemoryStore& InMemoryStore::instance() {
    static InMemoryStore _store;
    return _store;
}

InMemoryStore::InMemoryStore() {
    m_user = {"demo@docshift.local", "Chen Lu", "", false};
    m_tasks.push_back({kDemoTaskId, "默认工作区", {0, 0, 0, 0}});
    m_items.push_back(makeDemoItem(
        "a72d45bc-3dfd-4bf1-bfb0-f3703a630001",
        "项目说明.docx",
        "docx",
        245760,
        models::ConversionStatus::kPreviewReady
    ));
    m_items.push_back(makeDemoItem(
        "a72d45bc-3dfd-4bf1-bfb0-f3703a630002",
        "季度汇报.pptx",
        "pptx",
        8388608,
        models::ConversionStatus::kProcessing
    ));
    auto _failed_item = makeDemoItem(
        "a72d45bc-3dfd-4bf1-bfb0-f3703a630003",
        "损坏的附件.pdf",
        "pdf",
        1048576,
        models::ConversionStatus::kFailed
    );
    _failed_item.has_error = true;
    _failed_item.error = {"SOURCE_FILE_CORRUPTED", "文件损坏，无法解析。"};
    m_items.push_back(std::move(_failed_item));
}

models::UserProfile InMemoryStore::currentUser() const {
    std::lock_guard<std::mutex> _lock(m_mutex);
    return m_user;
}

models::UserProfile InMemoryStore::updateUser(
    const std::string& username,
    const std::string& avatar_url,
    const bool has_avatar
) {
    std::lock_guard<std::mutex> _lock(m_mutex);
    m_user.username = username;
    m_user.avatar_url = avatar_url;
    m_user.has_avatar = has_avatar;
    return m_user;
}

models::UserProfile InMemoryStore::login(const std::string& email) {
    std::lock_guard<std::mutex> _lock(m_mutex);
    m_user.email = email;
    return m_user;
}

std::vector<models::TaskSummary> InMemoryStore::taskSummaries() const {
    std::lock_guard<std::mutex> _lock(m_mutex);
    std::vector<models::TaskSummary> _tasks = m_tasks;
    for (std::vector<models::TaskSummary>::iterator _it = _tasks.begin(); _it != _tasks.end(); ++_it) {
        _it->counts = taskCounts(_it->task_id);
    }
    return _tasks;
}

models::TaskSummary InMemoryStore::createTask(const std::string& display_name) {
    std::lock_guard<std::mutex> _lock(m_mutex);
    models::TaskSummary _task;
    _task.task_id = nextId("task");
    _task.display_name = display_name;
    _task.counts = {0, 0, 0, 0};
    m_tasks.insert(m_tasks.begin(), _task);
    return _task;
}

bool InMemoryStore::findTask(const std::string& task_id, models::TaskSummary* task) const {
    std::lock_guard<std::mutex> _lock(m_mutex);
    const std::vector<models::TaskSummary>::const_iterator _it = std::find_if(
        m_tasks.begin(),
        m_tasks.end(),
        [&task_id](const models::TaskSummary& task_value) {
            return task_value.task_id == task_id;
        }
    );
    if (_it == m_tasks.end()) {
        return false;
    }

    *task = *_it;
    task->counts = taskCounts(task_id);
    return true;
}

std::vector<models::ConversionItem> InMemoryStore::conversionItems() const {
    return conversionItems("");
}

std::vector<models::ConversionItem> InMemoryStore::conversionItems(const std::string& task_id) const {
    std::lock_guard<std::mutex> _lock(m_mutex);
    std::vector<models::ConversionItem> _items;
    for (const auto& _item : m_items) {
        if (_item.status != models::ConversionStatus::kDeleted
            && (task_id.empty() || _item.task_id == task_id)) {
            _items.push_back(_item);
        }
    }
    return _items;
}

models::ConversionItem InMemoryStore::createConversion(
    const std::string& item_id,
    const std::string& task_id,
    const std::string& filename,
    const std::uint64_t size_bytes,
    const std::string& target_format,
    const std::string& storage_path
) {
    std::lock_guard<std::mutex> _lock(m_mutex);
    models::ConversionItem _item;
    _item.item_id = item_id;
    _item.task_id = task_id;
    _item.source = {filename, detectFormat(filename), size_bytes, storage_path};
    _item.target_format = target_format;
    _item.status = models::ConversionStatus::kProcessing;
    _item.download_available = false;
    _item.has_error = false;
    m_items.insert(m_items.begin(), _item);
    return _item;
}

std::string InMemoryStore::reserveConversionId() {
    std::lock_guard<std::mutex> _lock(m_mutex);
    return nextId("conversion");
}

models::TaskCounts InMemoryStore::taskCounts(const std::string& task_id) const {
    models::TaskCounts _counts = {0, 0, 0, 0};
    for (std::vector<models::ConversionItem>::const_iterator _it = m_items.begin(); _it != m_items.end(); ++_it) {
        if (_it->task_id != task_id || _it->status == models::ConversionStatus::kDeleted) {
            continue;
        }

        ++_counts.total;
        switch (_it->status) {
        case models::ConversionStatus::kQueued:
        case models::ConversionStatus::kProcessing:
            ++_counts.processing;
            break;
        case models::ConversionStatus::kPreviewReady:
            ++_counts.preview_ready;
            break;
        case models::ConversionStatus::kFailed:
            ++_counts.failed;
            break;
        case models::ConversionStatus::kExpired:
        case models::ConversionStatus::kDeleted:
            break;
        }
    }
    return _counts;
}

bool InMemoryStore::findConversion(const std::string& item_id, models::ConversionItem* item) const {
    std::lock_guard<std::mutex> _lock(m_mutex);
    const auto _iterator = std::find_if(m_items.begin(), m_items.end(), [&item_id](const models::ConversionItem& item_value) {
        return item_value.item_id == item_id && item_value.status != models::ConversionStatus::kDeleted;
    });
    if (_iterator == m_items.end()) {
        return false;
    }

    *item = *_iterator;
    return true;
}

bool InMemoryStore::deleteConversion(const std::string& item_id) {
    std::lock_guard<std::mutex> _lock(m_mutex);
    const auto _iterator = std::find_if(m_items.begin(), m_items.end(), [&item_id](const models::ConversionItem& item_value) {
        return item_value.item_id == item_id && item_value.status != models::ConversionStatus::kDeleted;
    });
    if (_iterator == m_items.end()) {
        return false;
    }

    _iterator->status = models::ConversionStatus::kDeleted;
    _iterator->download_available = false;
    return true;
}

models::TaskExport InMemoryStore::createTaskExport(const std::string& task_id) {
    std::lock_guard<std::mutex> _lock(m_mutex);
    std::vector<models::ConversionItem> _items;
    for (const auto& _item : m_items) {
        if (_item.task_id == task_id && _item.status == models::ConversionStatus::kPreviewReady && _item.download_available) {
            _items.push_back(_item);
        }
    }
    if (_items.empty()) {
        throw common::ApiError(
            409,
            "NO_EXPORTABLE_ITEMS",
            "当前任务没有可导出的文件。"
        );
    }

    const std::string _export_id = nextId("export");
    m_exports.emplace(_export_id, _items);
    return {_export_id, "docshift-export.zip", _items.size(), "/exports/" + _export_id + "/download"};
}

bool InMemoryStore::findExport(const std::string& export_id, std::vector<models::ConversionItem>* items) const {
    std::lock_guard<std::mutex> _lock(m_mutex);
    const auto _iterator = m_exports.find(export_id);
    if (_iterator == m_exports.end()) {
        return false;
    }

    *items = _iterator->second;
    return true;
}

std::string InMemoryStore::nextId(const std::string& prefix) {
    return prefix + "-" + std::to_string(m_next_id++);
}

std::string InMemoryStore::detectFormat(const std::string& filename) {
    const std::string::size_type _position = filename.find_last_of('.');
    if (_position == std::string::npos || _position + 1 == filename.size()) {
        return "file";
    }

    std::string _format = filename.substr(_position + 1);
    std::transform(_format.begin(), _format.end(), _format.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return _format;
}

} // namespace storage
} // namespace docshift
