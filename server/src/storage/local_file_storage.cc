#include <storage/local_file_storage.hxx>

#include <cerrno>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <common/api_error.hxx>

namespace docshift {
namespace storage {
namespace {

bool createDirectory(const std::string& path) {
#ifdef _WIN32
    const int _result = _mkdir(path.c_str());
#else
    const int _result = mkdir(path.c_str(), 0750);
#endif
    return _result == 0 || errno == EEXIST;
}

void createDirectories(const std::string& path) {
    std::string _current;
    std::size_t _position = 0;
    if (!path.empty() && path[0] == '/') {
        _current = "/";
        _position = 1;
    } else if (path.size() > 1 && path[1] == ':') {
        _current = path.substr(0, 2);
        _position = 2;
    }

    while (_position <= path.size()) {
        const std::size_t _next = path.find('/', _position);
        const std::string _part = path.substr(_position, _next - _position);
        if (!_part.empty() && _part != ".") {
            if (_part == "..") {
                throw common::ApiError(500, "DATA_ROOT_INVALID", "数据目录配置无效。");
            }
            if (!_current.empty() && _current[_current.size() - 1] != '/') {
                _current += "/";
            }
            _current += _part;
            if (!createDirectory(_current)) {
                throw common::ApiError(500, "FILE_STORAGE_FAILED", "无法创建文件存储目录。");
            }
        }
        if (_next == std::string::npos) {
            break;
        }
        _position = _next + 1;
    }
}

std::string filenameForStorage(const std::string& filename) {
    const std::size_t _separator = filename.find_last_of("/\\");
    const std::string _basename = _separator == std::string::npos ? filename : filename.substr(_separator + 1);
    if (_basename.empty() || _basename == "." || _basename == "..") {
        throw common::ApiError(422, "FILENAME_INVALID", "文件名不能为空。" );
    }

    std::string _result;
    _result.reserve(_basename.size());
    for (std::string::const_iterator _it = _basename.begin(); _it != _basename.end() && _result.size() < 128; ++_it) {
        const unsigned char _character = static_cast<unsigned char>(*_it);
        const bool _is_safe = (_character >= 'a' && _character <= 'z')
            || (_character >= 'A' && _character <= 'Z')
            || (_character >= '0' && _character <= '9')
            || _character == '.' || _character == '_' || _character == '-';
        _result += _is_safe ? static_cast<char>(_character) : '_';
    }
    if (_result.empty()) {
        throw common::ApiError(422, "FILENAME_INVALID", "文件名不能为空。" );
    }
    return _result;
}

std::string userDirectoryName(const std::string& email) {
    unsigned long long _hash = 1469598103934665603ULL;
    for (std::string::const_iterator _it = email.begin(); _it != email.end(); ++_it) {
        _hash ^= static_cast<unsigned char>(*_it);
        _hash *= 1099511628211ULL;
    }
    std::ostringstream _stream;
    _stream << std::hex << _hash;
    return "user-" + _stream.str();
}

} // namespace

LocalFileStorage::LocalFileStorage(const std::string& data_root)
    : m_data_root(data_root) {
}

std::string LocalFileStorage::saveSourceFile(
    const std::string& user_email,
    const std::string& task_id,
    const std::string& item_id,
    const std::string& filename,
    const std::string& content
) const {
    const std::string _storage_filename = item_id + "-" + filenameForStorage(filename);
    const std::string _directory = m_data_root + "/users/" + userDirectoryName(user_email)
        + "/tasks/" + task_id + "/sources";
    createDirectories(_directory);

    const std::string _storage_path = _directory + "/" + _storage_filename;
    std::ofstream _output(_storage_path.c_str(), std::ios::binary | std::ios::trunc);
    if (!_output.is_open()) {
        throw common::ApiError(500, "FILE_STORAGE_FAILED", "无法写入上传文件。" );
    }
    _output.write(content.data(), static_cast<std::streamsize>(content.size()));
    _output.close();
    if (!_output) {
        std::remove(_storage_path.c_str());
        throw common::ApiError(500, "FILE_STORAGE_FAILED", "上传文件写入失败。" );
    }
    return _storage_path;
}

void LocalFileStorage::removeSourceFile(const std::string& storage_path) const {
    if (storage_path.empty()) {
        return;
    }
    if (std::remove(storage_path.c_str()) != 0 && errno != ENOENT) {
        throw common::ApiError(500, "FILE_DELETE_FAILED", "无法删除上传文件。" );
    }
}

} // namespace storage
} // namespace docshift
