#include <services/export_service.hxx>

#include <cstdint>
#include <string>
#include <vector>

#include <common/api_error.hxx>
#include <storage/in_memory_store.hxx>

namespace docshift {
namespace services {
namespace {

void appendUint16(std::string* output, const std::uint16_t value) {
    output->push_back(static_cast<char>(value & 0xff));
    output->push_back(static_cast<char>((value >> 8) & 0xff));
}

void appendUint32(std::string* output, const std::uint32_t value) {
    appendUint16(output, static_cast<std::uint16_t>(value & 0xffff));
    appendUint16(output, static_cast<std::uint16_t>((value >> 16) & 0xffff));
}

std::uint32_t crc32(const std::string& content) {
    std::uint32_t _crc = 0xffffffff;
    for (const unsigned char character : content) {
        _crc ^= character;
        for (int _bit = 0; _bit < 8; ++_bit) {
            _crc = (_crc & 1U) == 0U ? _crc >> 1U : (_crc >> 1U) ^ 0xedb88320U;
        }
    }
    return _crc ^ 0xffffffff;
}

std::string exportName(const models::ConversionItem& item) {
    const std::string::size_type _position = item.source.filename.find_last_of('.');
    const std::string _stem = _position == std::string::npos
        ? item.source.filename
        : item.source.filename.substr(0, _position);
    const std::string _extension = item.target_format == "markdown" ? "md" : item.target_format;
    return _stem + "." + _extension;
}

std::string exportContent(const models::ConversionItem& item) {
    return "# " + item.source.filename + "\n\n"
        "这是由 DocShift C++ 服务生成的演示导出内容。\n\n"
        "- 源格式：" + item.source.format + "\n"
        "- 目标格式：" + item.target_format + "\n"
        "- 转换状态：已完成\n";
}

} // namespace

models::ExportArchive ExportService::buildArchive(const std::string& export_id) const {
    std::vector<models::ConversionItem> _items;
    if (!storage::InMemoryStore::instance().findExport(export_id, &_items)) {
        throw common::ApiError(404, "EXPORT_NOT_FOUND", "导出记录不存在或已过期。");
    }

    struct CentralDirectoryEntry {
        std::string filename;
        std::uint32_t crc;
        std::uint32_t size;
        std::uint32_t offset;
    };

    std::string _archive;
    std::vector<CentralDirectoryEntry> _entries;
    for (const auto& _item : _items) {
        const std::string _filename = exportName(_item);
        const std::string _content = exportContent(_item);
        const std::uint32_t _offset = static_cast<std::uint32_t>(_archive.size());
        const std::uint32_t _crc = crc32(_content);
        const std::uint32_t _size = static_cast<std::uint32_t>(_content.size());

        appendUint32(&_archive, 0x04034b50U);
        appendUint16(&_archive, 20);
        appendUint16(&_archive, 0x0800);
        appendUint16(&_archive, 0);
        appendUint16(&_archive, 0);
        appendUint16(&_archive, 0);
        appendUint32(&_archive, _crc);
        appendUint32(&_archive, _size);
        appendUint32(&_archive, _size);
        appendUint16(&_archive, static_cast<std::uint16_t>(_filename.size()));
        appendUint16(&_archive, 0);
        _archive += _filename;
        _archive += _content;
        _entries.push_back({_filename, _crc, _size, _offset});
    }

    const std::uint32_t _central_offset = static_cast<std::uint32_t>(_archive.size());
    for (const auto& _entry : _entries) {
        appendUint32(&_archive, 0x02014b50U);
        appendUint16(&_archive, 20);
        appendUint16(&_archive, 20);
        appendUint16(&_archive, 0x0800);
        appendUint16(&_archive, 0);
        appendUint16(&_archive, 0);
        appendUint16(&_archive, 0);
        appendUint32(&_archive, _entry.crc);
        appendUint32(&_archive, _entry.size);
        appendUint32(&_archive, _entry.size);
        appendUint16(&_archive, static_cast<std::uint16_t>(_entry.filename.size()));
        appendUint16(&_archive, 0);
        appendUint16(&_archive, 0);
        appendUint16(&_archive, 0);
        appendUint16(&_archive, 0);
        appendUint32(&_archive, 0);
        appendUint32(&_archive, _entry.offset);
        _archive += _entry.filename;
    }

    const std::uint32_t _central_size = static_cast<std::uint32_t>(_archive.size()) - _central_offset;
    appendUint32(&_archive, 0x06054b50U);
    appendUint16(&_archive, 0);
    appendUint16(&_archive, 0);
    appendUint16(&_archive, static_cast<std::uint16_t>(_entries.size()));
    appendUint16(&_archive, static_cast<std::uint16_t>(_entries.size()));
    appendUint32(&_archive, _central_size);
    appendUint32(&_archive, _central_offset);
    appendUint16(&_archive, 0);

    return {"docshift-export.zip", std::move(_archive)};
}

} // namespace services
} // namespace docshift
