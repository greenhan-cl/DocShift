#ifndef DOCSHIFT_MODELS_CONVERSION_ITEM_HXX
#define DOCSHIFT_MODELS_CONVERSION_ITEM_HXX

#include <cstdint>
#include <string>

namespace docshift {
namespace models {

enum class ConversionStatus {
    kQueued = 0,
    kProcessing,
    kPreviewReady,
    kFailed,
    kExpired,
    kDeleted
};

struct SourceFile {
    std::string filename;
    std::string format;
    std::uint64_t size_bytes;
    std::string storage_path;
};

struct ConversionError {
    std::string code;
    std::string message;
};

struct ConversionItem {
    std::string item_id;
    std::string task_id;
    SourceFile source;
    std::string target_format;
    ConversionStatus status;
    bool download_available;
    bool has_error;
    ConversionError error;
};

struct Preview {
    std::string title;
    std::string kind;
    std::string content;
};

} // namespace models
} // namespace docshift

#endif
