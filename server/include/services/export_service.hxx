#ifndef DOCSHIFT_SERVICES_EXPORT_SERVICE_HXX
#define DOCSHIFT_SERVICES_EXPORT_SERVICE_HXX

#include <string>

#include <models/task_export.hxx>

namespace docshift {
namespace services {

class ExportService {
public:
    models::ExportArchive buildArchive(const std::string& export_id) const;
};

} // namespace services
} // namespace docshift

#endif
