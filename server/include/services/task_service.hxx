#ifndef DOCSHIFT_SERVICES_TASK_SERVICE_HXX
#define DOCSHIFT_SERVICES_TASK_SERVICE_HXX

#include <string>
#include <vector>

#include <models/conversion_item.hxx>
#include <models/task_summary.hxx>

namespace docshift {
namespace services {

class TaskService {
public:
    std::vector<models::TaskSummary> list() const;
    models::TaskSummary create(const std::string& display_name) const;
    models::TaskSummary find(const std::string& task_id) const;
    std::vector<models::ConversionItem> conversionItems(const std::string& task_id) const;
};

} // namespace services
} // namespace docshift

#endif
