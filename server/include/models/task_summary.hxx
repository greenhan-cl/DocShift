#ifndef DOCSHIFT_MODELS_TASK_SUMMARY_HXX
#define DOCSHIFT_MODELS_TASK_SUMMARY_HXX

#include <cstddef>
#include <string>

namespace docshift {
namespace models {

struct TaskCounts {
    std::size_t total;
    std::size_t processing;
    std::size_t preview_ready;
    std::size_t failed;
};

struct TaskSummary {
    std::string task_id;
    std::string display_name;
    TaskCounts counts;
};

} // namespace models
} // namespace docshift

#endif
