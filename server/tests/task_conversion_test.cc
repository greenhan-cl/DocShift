#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include <common/api_error.hxx>
#include <models/conversion_item.hxx>
#include <services/conversion_service.hxx>
#include <services/task_service.hxx>

namespace docshift {
namespace {

template <typename Callable>
void expectApiError(const Callable& callable, const int status_code, const std::string& error_code) {
    try {
        callable();
        ADD_FAILURE() << "Expected ApiError was not thrown.";
    } catch (const common::ApiError& _error) {
        EXPECT_EQ(status_code, _error.statusCode());
        EXPECT_EQ(error_code, _error.errorCode());
    }
}

TEST(TaskServiceTest, CreateTaskCanBeFoundWithEmptyCounts) {
    services::TaskService _task_service;

    const models::TaskSummary _created_task = _task_service.create("测试任务");
    const models::TaskSummary _found_task = _task_service.find(_created_task.task_id);

    EXPECT_EQ("测试任务", _found_task.display_name);
    EXPECT_EQ(0U, _found_task.counts.total);
    EXPECT_EQ(0U, _found_task.counts.processing);
    EXPECT_EQ(0U, _found_task.counts.preview_ready);
    EXPECT_EQ(0U, _found_task.counts.failed);
}

TEST(TaskServiceTest, RejectsMissingAndOverlongTaskNames) {
    services::TaskService _task_service;
    const std::string _overlong_name(65, 'a');

    expectApiError([&_task_service]() { _task_service.find("task-does-not-exist"); }, 404, "TASK_NOT_FOUND");
    expectApiError(
        [&_task_service, &_overlong_name]() { _task_service.create(_overlong_name); },
        422,
        "TASK_NAME_TOO_LONG"
    );
}

TEST(ConversionServiceTest, CreatesConversionOnlyInRequestedTask) {
    services::TaskService _task_service;
    services::ConversionService _conversion_service;
    const models::TaskSummary _first_task = _task_service.create("任务 A");
    const models::TaskSummary _second_task = _task_service.create("任务 B");

    const models::ConversionItem _item = _conversion_service.create(
        _first_task.task_id,
        "UPPER.DOCX",
        static_cast<std::uint64_t>(1024),
        "markdown"
    );

    EXPECT_EQ(_first_task.task_id, _item.task_id);
    EXPECT_EQ("docx", _item.source.format);
    EXPECT_EQ(1U, _task_service.conversionItems(_first_task.task_id).size());
    EXPECT_TRUE(_task_service.conversionItems(_second_task.task_id).empty());
    EXPECT_EQ(1U, _task_service.find(_first_task.task_id).counts.processing);
}

TEST(ConversionServiceTest, RejectsInvalidConversionInput) {
    services::TaskService _task_service;
    services::ConversionService _conversion_service;
    const models::TaskSummary _task = _task_service.create("校验任务");

    expectApiError(
        [&_conversion_service, &_task]() { _conversion_service.create(_task.task_id, "", 1, "markdown"); },
        422,
        "FILENAME_INVALID"
    );
    expectApiError(
        [&_conversion_service, &_task]() { _conversion_service.create(_task.task_id, "a.docx", 1, ""); },
        422,
        "TARGET_FORMAT_INVALID"
    );
    expectApiError(
        [&_conversion_service]() { _conversion_service.create("task-does-not-exist", "a.docx", 1, "markdown"); },
        404,
        "TASK_NOT_FOUND"
    );
}

TEST(ConversionServiceTest, RemovesItemAndPreventsSecondRemoval) {
    services::TaskService _task_service;
    services::ConversionService _conversion_service;
    const models::TaskSummary _task = _task_service.create("删除任务");
    const models::ConversionItem _item = _conversion_service.create(
        _task.task_id,
        "remove.docx",
        static_cast<std::uint64_t>(1),
        "markdown"
    );

    _conversion_service.remove(_item.item_id);

    EXPECT_TRUE(_task_service.conversionItems(_task.task_id).empty());
    EXPECT_EQ(0U, _task_service.find(_task.task_id).counts.total);
    expectApiError([&_conversion_service, &_item]() { _conversion_service.remove(_item.item_id); }, 404, "ITEM_NOT_FOUND");
}

} // namespace
} // namespace docshift
