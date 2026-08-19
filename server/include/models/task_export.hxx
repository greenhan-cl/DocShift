#ifndef DOCSHIFT_MODELS_TASK_EXPORT_HXX
#define DOCSHIFT_MODELS_TASK_EXPORT_HXX

#include <cstddef>
#include <string>

namespace docshift {
namespace models {

struct TaskExport {
    std::string export_id;
    std::string filename;
    std::size_t item_count;
    std::string download_url;
};

struct ExportArchive {
    std::string filename;
    std::string content;
};

} // namespace models
} // namespace docshift

#endif
