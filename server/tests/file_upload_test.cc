#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

#include <common/api_error.hxx>
#include <services/conversion_service.hxx>
#include <services/task_service.hxx>

namespace docshift {
namespace {

std::string testDataRoot(const std::string& test_name) {
    return std::string(::testing::TempDir()) + "docshift-" + test_name;
}

std::string readFile(const std::string& path) {
    std::ifstream _input(path.c_str(), std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(_input),
        std::istreambuf_iterator<char>()
    );
}

TEST(FileUploadTest, StoresUploadedFileAndDeletesItWithConversion) {
    services::TaskService _task_service;
    services::ConversionService _conversion_service;
    const models::TaskSummary _task = _task_service.create("上传测试任务");
    const std::string _content = "DocShift upload test\n";
    const std::string _data_root = testDataRoot("store-and-delete");

    const models::ConversionItem _item = _conversion_service.createUploaded(
        _data_root,
        "upload-test@example.com",
        _task.task_id,
        "meeting notes.DOCX",
        _content,
        "markdown",
        1024
    );

    EXPECT_EQ("docx", _item.source.format);
    EXPECT_EQ(_content.size(), _item.source.size_bytes);
    EXPECT_NE(std::string::npos, _item.source.storage_path.find("/sources/"));
    EXPECT_EQ(_content, readFile(_item.source.storage_path));

    _conversion_service.remove(_data_root, _item.item_id);

    std::ifstream _input(_item.source.storage_path.c_str(), std::ios::binary);
    EXPECT_FALSE(_input.good());
}

TEST(FileUploadTest, SanitizesFilenameBeforeWritingToDataDirectory) {
    services::TaskService _task_service;
    services::ConversionService _conversion_service;
    const models::TaskSummary _task = _task_service.create("文件名测试任务");
    const std::string _data_root = testDataRoot("sanitize-filename");

    const models::ConversionItem _item = _conversion_service.createUploaded(
        _data_root,
        "upload-test@example.com",
        _task.task_id,
        "../../unsafe file.docx",
        "content",
        "markdown",
        1024
    );

    EXPECT_EQ(std::string::npos, _item.source.storage_path.find(".."));
    EXPECT_NE(std::string::npos, _item.source.storage_path.find("unsafe_file.docx"));
    _conversion_service.remove(_data_root, _item.item_id);
}

TEST(FileUploadTest, RejectsContentOverConfiguredLimit) {
    services::TaskService _task_service;
    services::ConversionService _conversion_service;
    const models::TaskSummary _task = _task_service.create("大小限制测试任务");

    try {
        _conversion_service.createUploaded(
            testDataRoot("too-large"),
            "upload-test@example.com",
            _task.task_id,
            "large.docx",
            "12345",
            "markdown",
            4
        );
        ADD_FAILURE() << "Expected upload limit error was not thrown.";
    } catch (const common::ApiError& _error) {
        EXPECT_EQ(413, _error.statusCode());
        EXPECT_EQ("UPLOAD_LIMIT_EXCEEDED", _error.errorCode());
    }
}

} // namespace
} // namespace docshift
