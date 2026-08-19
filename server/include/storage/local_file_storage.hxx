#ifndef DOCSHIFT_STORAGE_LOCAL_FILE_STORAGE_HXX
#define DOCSHIFT_STORAGE_LOCAL_FILE_STORAGE_HXX

#include <string>

namespace docshift {
namespace storage {

/**
 * Stores uploaded source files below the configured local data root.
 */
class LocalFileStorage {
public:
    explicit LocalFileStorage(const std::string& data_root);

    std::string saveSourceFile(
        const std::string& user_email,
        const std::string& task_id,
        const std::string& item_id,
        const std::string& filename,
        const std::string& content
    ) const;
    void removeSourceFile(const std::string& storage_path) const;

private:
    std::string m_data_root;
};

} // namespace storage
} // namespace docshift

#endif
