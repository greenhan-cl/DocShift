#ifndef DOCSHIFT_SERVICES_CONVERSION_SERVICE_HXX
#define DOCSHIFT_SERVICES_CONVERSION_SERVICE_HXX

#include <cstdint>
#include <string>
#include <vector>

#include <models/conversion_item.hxx>
#include <models/task_export.hxx>

namespace docshift {
namespace services {

class ConversionService {
public:
    std::vector<models::ConversionItem> list() const;
    models::ConversionItem create(const std::string& task_id, const std::string& filename, std::uint64_t size_bytes, const std::string& target_format) const;
    models::ConversionItem createUploaded(
        const std::string& data_root,
        const std::string& user_email,
        const std::string& task_id,
        const std::string& filename,
        const std::string& content,
        const std::string& target_format,
        std::uint64_t upload_max_bytes
    ) const;
    models::Preview preview(const std::string& item_id) const;
    void remove(const std::string& item_id) const;
    void remove(const std::string& data_root, const std::string& item_id) const;
    models::TaskExport createTaskExport(const std::string& task_id) const;
};

} // namespace services
} // namespace docshift

#endif
