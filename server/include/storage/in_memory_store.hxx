#ifndef DOCSHIFT_STORAGE_IN_MEMORY_STORE_HXX
#define DOCSHIFT_STORAGE_IN_MEMORY_STORE_HXX

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <models/conversion_item.hxx>
#include <models/task_export.hxx>
#include <models/task_summary.hxx>
#include <models/user_profile.hxx>

namespace docshift {
namespace storage {

class InMemoryStore {
public:
    static InMemoryStore& instance();

    models::UserProfile currentUser() const;
    models::UserProfile updateUser(const std::string& username, const std::string& avatar_url, bool has_avatar);
    models::UserProfile login(const std::string& email);

    std::vector<models::TaskSummary> taskSummaries() const;
    models::TaskSummary createTask(const std::string& display_name);
    bool findTask(const std::string& task_id, models::TaskSummary* task) const;

    std::vector<models::ConversionItem> conversionItems() const;
    std::vector<models::ConversionItem> conversionItems(const std::string& task_id) const;
    std::string reserveConversionId();
    models::ConversionItem createConversion(
        const std::string& item_id,
        const std::string& task_id,
        const std::string& filename,
        std::uint64_t size_bytes,
        const std::string& target_format,
        const std::string& storage_path
    );
    bool findConversion(const std::string& item_id, models::ConversionItem* item) const;
    bool deleteConversion(const std::string& item_id);

    models::TaskExport createTaskExport(const std::string& task_id);
    bool findExport(const std::string& export_id, std::vector<models::ConversionItem>* items) const;

private:
    InMemoryStore();

    models::TaskCounts taskCounts(const std::string& task_id) const;
    std::string nextId(const std::string& prefix);
    static std::string detectFormat(const std::string& filename);

    mutable std::mutex m_mutex;
    models::UserProfile m_user;
    std::vector<models::TaskSummary> m_tasks;
    std::vector<models::ConversionItem> m_items;
    std::unordered_map<std::string, std::vector<models::ConversionItem>> m_exports;
    std::uint64_t m_next_id = 1;
};

} // namespace storage
} // namespace docshift

#endif
