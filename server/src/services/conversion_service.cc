#include <services/conversion_service.hxx>

#include <common/api_error.hxx>
#include <common/json_serialization.hxx>
#include <storage/local_file_storage.hxx>
#include <storage/in_memory_store.hxx>

namespace docshift {
namespace services {

std::vector<models::ConversionItem> ConversionService::list() const {
    return storage::InMemoryStore::instance().conversionItems();
}

models::ConversionItem ConversionService::create(
    const std::string& task_id,
    const std::string& filename,
    const std::uint64_t size_bytes,
    const std::string& target_format
) const {
    models::TaskSummary _task;
    if (!storage::InMemoryStore::instance().findTask(task_id, &_task)) {
        throw common::ApiError(404, "TASK_NOT_FOUND", "Task 不存在。");
    }
    if (filename.empty()) {
        throw common::ApiError(422, "FILENAME_INVALID", "文件名不能为空。");
    }
    if (target_format.empty()) {
        throw common::ApiError(422, "TARGET_FORMAT_INVALID", "目标格式不能为空。");
    }

    storage::InMemoryStore& _store = storage::InMemoryStore::instance();
    return _store.createConversion(
        _store.reserveConversionId(),
        task_id,
        filename,
        size_bytes,
        target_format,
        ""
    );
}

models::ConversionItem ConversionService::createUploaded(
    const std::string& data_root,
    const std::string& user_email,
    const std::string& task_id,
    const std::string& filename,
    const std::string& content,
    const std::string& target_format,
    const std::uint64_t upload_max_bytes
) const {
    if (content.size() > upload_max_bytes) {
        throw common::ApiError(413, "UPLOAD_LIMIT_EXCEEDED", "上传文件超过大小限制。" );
    }

    models::TaskSummary _task;
    if (!storage::InMemoryStore::instance().findTask(task_id, &_task)) {
        throw common::ApiError(404, "TASK_NOT_FOUND", "Task 不存在。");
    }
    if (filename.empty()) {
        throw common::ApiError(422, "FILENAME_INVALID", "文件名不能为空。");
    }
    if (target_format.empty()) {
        throw common::ApiError(422, "TARGET_FORMAT_INVALID", "目标格式不能为空。");
    }

    storage::InMemoryStore& _store = storage::InMemoryStore::instance();
    const std::string _item_id = _store.reserveConversionId();
    storage::LocalFileStorage _file_storage(data_root);
    const std::string _storage_path = _file_storage.saveSourceFile(
        user_email,
        task_id,
        _item_id,
        filename,
        content
    );
    try {
        return _store.createConversion(
            _item_id,
            task_id,
            filename,
            static_cast<std::uint64_t>(content.size()),
            target_format,
            _storage_path
        );
    } catch (...) {
        _file_storage.removeSourceFile(_storage_path);
        throw;
    }
}

models::Preview ConversionService::preview(const std::string& item_id) const {
    models::ConversionItem _item;
    if (!storage::InMemoryStore::instance().findConversion(item_id, &_item)) {
        throw common::ApiError(404, "CONVERSION_NOT_FOUND", "转换文件不存在。");
    }

    if (_item.status == models::ConversionStatus::kPreviewReady) {
        return {
            _item.source.filename + " · 转换结果",
            "result",
            "# " + _item.source.filename + "\n\n这是由 DocShift C++ 服务生成的 **" + _item.target_format
                + "** 演示预览内容。\n\n- 源格式：" + _item.source.format + "\n- 转换状态：已完成\n- 可在此处确认结果后导出。"
        };
    }

    return {
        _item.source.filename,
        "source",
        "源文件预览\n\n文件名：" + _item.source.filename + "\n格式：" + _item.source.format
            + "\n大小：" + std::to_string(_item.source.size_bytes) + " bytes\n\n当前状态："
            + common::toStatusText(_item.status) + "。转换完成后将在此显示结果预览。"
    };
}

void ConversionService::remove(const std::string& item_id) const {
    storage::InMemoryStore& _store = storage::InMemoryStore::instance();
    if (!_store.deleteConversion(item_id)) {
        throw common::ApiError(404, "ITEM_NOT_FOUND", "转换项不存在。");
    }
}

void ConversionService::remove(const std::string& data_root, const std::string& item_id) const {
    storage::InMemoryStore& _store = storage::InMemoryStore::instance();
    models::ConversionItem _item;
    if (!_store.findConversion(item_id, &_item)) {
        throw common::ApiError(404, "ITEM_NOT_FOUND", "转换项不存在。");
    }
    storage::LocalFileStorage(data_root).removeSourceFile(_item.source.storage_path);
    _store.deleteConversion(item_id);
}

models::TaskExport ConversionService::createTaskExport(const std::string& task_id) const {
    return storage::InMemoryStore::instance().createTaskExport(task_id);
}

} // namespace services
} // namespace docshift
