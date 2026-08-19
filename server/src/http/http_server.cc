#include <http/http_server.hxx>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <common/api_error.hxx>
#include <common/json_serialization.hxx>
#include <httplib/httplib.h>
#include <services/auth_service.hxx>
#include <services/conversion_service.hxx>
#include <services/export_service.hxx>
#include <services/task_service.hxx>
#include <services/user_service.hxx>

namespace docshift {
namespace http {
namespace {

void writeJson(httplib::Response* response, const nlohmann::json& body, const int status) {
    response->status = status;
    response->set_content(body.dump(), "application/json; charset=utf-8");
}

void writeData(httplib::Response* response, const nlohmann::json& data, const int status) {
    nlohmann::json _body;
    _body["data"] = data;
    writeJson(response, _body, status);
}

void writeError(httplib::Response* response, const common::ApiError& error) {
    nlohmann::json _body;
    _body["error"] = {{"code", error.errorCode()}, {"message", error.what()}};
    writeJson(response, _body, error.statusCode());
}

nlohmann::json requestJson(const httplib::Request& request) {
    try {
        return nlohmann::json::parse(request.body);
    } catch (const nlohmann::json::exception&) {
        throw common::ApiError(400, "REQUEST_BODY_INVALID", "请求体必须是 JSON 对象。");
    }
}

std::string requiredString(const nlohmann::json& body, const char* name) {
    if (!body.is_object() || !body.count(name) || !body[name].is_string()) {
        throw common::ApiError(422, "REQUEST_FIELD_INVALID", std::string(name) + " 必须是字符串。");
    }
    return body[name].get<std::string>();
}

std::string optionalString(const nlohmann::json& body, const char* name) {
    if (!body.is_object()) {
        throw common::ApiError(422, "REQUEST_BODY_INVALID", "请求体必须是 JSON 对象。");
    }
    if (!body.count(name) || body[name].is_null()) {
        return "";
    }
    if (!body[name].is_string()) {
        throw common::ApiError(422, "REQUEST_FIELD_INVALID", std::string(name) + " 必须是字符串。");
    }
    return body[name].get<std::string>();
}

std::string requiredPathParameter(const httplib::Request& request, const char* name) {
    const auto _it = request.path_params.find(name);
    if (_it == request.path_params.end() || _it->second.empty()) {
        throw common::ApiError(404, "RESOURCE_NOT_FOUND", "请求的资源不存在。");
    }

    return _it->second;
}

std::string requiredMultipartField(const httplib::Request& request, const char* name) {
    if (!request.is_multipart_form_data()) {
        throw common::ApiError(415, "UPLOAD_CONTENT_TYPE_INVALID", "上传请求必须使用 multipart/form-data。" );
    }
    if (!request.form.has_field(name)) {
        throw common::ApiError(422, "REQUEST_FIELD_INVALID", std::string(name) + " 必须是字符串。" );
    }
    const std::string _value = request.form.get_field(name);
    if (_value.empty()) {
        throw common::ApiError(422, "REQUEST_FIELD_INVALID", std::string(name) + " 不能为空。" );
    }
    return _value;
}

httplib::FormData requiredMultipartFile(const httplib::Request& request) {
    if (!request.is_multipart_form_data()) {
        throw common::ApiError(415, "UPLOAD_CONTENT_TYPE_INVALID", "上传请求必须使用 multipart/form-data。" );
    }
    if (!request.form.has_file("file") || request.form.get_file_count("file") != 1) {
        throw common::ApiError(422, "UPLOAD_FILE_REQUIRED", "必须上传一个文件。" );
    }
    const httplib::FormData _file = request.form.get_file("file");
    if (_file.filename.empty()) {
        throw common::ApiError(422, "FILENAME_INVALID", "文件名不能为空。" );
    }
    return _file;
}

} // namespace

HttpServer::HttpServer(const config::ServerConfiguration& configuration)
    : m_configuration(configuration) {
}

void HttpServer::run() {
    httplib::Server _server;
    const std::size_t _thread_count = m_configuration.threadCount();
    _server.new_task_queue = [_thread_count]() {
        return new httplib::ThreadPool(_thread_count);
    };
    const std::uint64_t _upload_max_bytes = m_configuration.uploadMaxBytes();
    const std::uint64_t _multipart_overhead_bytes = 1024ULL * 1024ULL;
    const std::uint64_t _payload_max_bytes = _upload_max_bytes + _multipart_overhead_bytes;
    _server.set_payload_max_length(static_cast<std::size_t>(_payload_max_bytes));
    _server.Get("/api/v1/health", [](const httplib::Request&, httplib::Response& response) {
        writeJson(&response, {{"service", "docshift-server"}, {"status", "ok"}, {"version", DOCSHIFT_SERVER_VERSION}}, 200);
    });
    _server.Post("/api/v1/auth/email-verifications", [](const httplib::Request& request, httplib::Response& response) {
        try { services::AuthService _service; _service.requestEmailVerification(requiredString(requestJson(request), "email")); writeData(&response, {{"demo_code", "123456"}}, 202); }
        catch (const common::ApiError& error) { writeError(&response, error); }
    });
    _server.Post("/api/v1/auth/login", [](const httplib::Request& request, httplib::Response& response) {
        try { const nlohmann::json _body = requestJson(request); services::AuthService _service; writeData(&response, {{"user", common::toJson(_service.login(requiredString(_body, "email"), requiredString(_body, "verification_code")))}}, 200); }
        catch (const common::ApiError& error) { writeError(&response, error); }
    });
    _server.Patch("/api/v1/users/me", [](const httplib::Request& request, httplib::Response& response) {
        try { const nlohmann::json _body = requestJson(request); const bool _has_avatar = _body.count("avatar_url") && _body["avatar_url"].is_string(); if (_body.count("avatar_url") && !_has_avatar && !_body["avatar_url"].is_null()) throw common::ApiError(422, "AVATAR_INVALID", "头像数据格式不正确。"); services::UserService _service; writeData(&response, {{"user", common::toJson(_service.updateProfile(requiredString(_body, "username"), _has_avatar ? _body["avatar_url"].get<std::string>() : "", _has_avatar))}}, 200); }
        catch (const common::ApiError& error) { writeError(&response, error); }
    });
    _server.Get("/api/v1/tasks", [](const httplib::Request&, httplib::Response& response) {
        services::TaskService _service;
        const std::vector<models::TaskSummary> _tasks = _service.list();
        nlohmann::json _items = nlohmann::json::array();
        for (std::vector<models::TaskSummary>::const_iterator _it = _tasks.begin(); _it != _tasks.end(); ++_it) {
            _items.push_back(common::toJson(*_it));
        }
        writeData(&response, {{"items", _items}}, 200);
    });
    _server.Post("/api/v1/tasks", [](const httplib::Request& request, httplib::Response& response) {
        try {
            services::TaskService _service;
            writeData(&response, common::toJson(_service.create(optionalString(requestJson(request), "display_name"))), 201);
        } catch (const common::ApiError& error) {
            writeError(&response, error);
        }
    });
    _server.Get("/api/v1/tasks/:task_id", [](const httplib::Request& request, httplib::Response& response) {
        try {
            const std::string _task_id = requiredPathParameter(request, "task_id");
            services::TaskService _service;
            const models::TaskSummary _task = _service.find(_task_id);
            const std::vector<models::ConversionItem> _list = _service.conversionItems(_task_id);
            nlohmann::json _items = nlohmann::json::array();
            for (std::vector<models::ConversionItem>::const_iterator _it = _list.begin(); _it != _list.end(); ++_it) {
                _items.push_back(common::toJson(*_it));
            }
            writeData(&response, {{"task", common::toJson(_task)}, {"items", _items}}, 200);
        } catch (const common::ApiError& error) {
            writeError(&response, error);
        }
    });
    const std::string _data_root = m_configuration.dataRoot();
    _server.Post("/api/v1/tasks/:task_id/items", [_data_root, _upload_max_bytes](const httplib::Request& request, httplib::Response& response) {
        try {
            const httplib::FormData _file = requiredMultipartFile(request);
            services::UserService _user_service;
            services::ConversionService _service;
            writeData(&response, common::toJson(_service.createUploaded(
                _data_root,
                _user_service.currentProfile().email,
                requiredPathParameter(request, "task_id"),
                _file.filename,
                _file.content,
                requiredMultipartField(request, "target_format"),
                _upload_max_bytes
            )), 202);
        } catch (const common::ApiError& error) {
            writeError(&response, error);
        }
    });
    _server.Get("/api/v1/conversions", [](const httplib::Request&, httplib::Response& response) {
        services::ConversionService _service; nlohmann::json _items = nlohmann::json::array(); const std::vector<models::ConversionItem> _list = _service.list(); for (std::vector<models::ConversionItem>::const_iterator _it = _list.begin(); _it != _list.end(); ++_it) _items.push_back(common::toJson(*_it)); writeData(&response, {{"items", _items}}, 200);
    });
    _server.Post("/api/v1/conversions", [](const httplib::Request& request, httplib::Response& response) {
        try { const nlohmann::json _body = requestJson(request); const std::uint64_t _size = _body.count("size_bytes") && _body["size_bytes"].is_number_unsigned() ? _body["size_bytes"].get<std::uint64_t>() : 0; services::TaskService _task_service; const std::vector<models::TaskSummary> _tasks = _task_service.list(); if (_tasks.empty()) throw common::ApiError(409, "TASK_REQUIRED", "请先创建 Task。"); services::ConversionService _service; writeData(&response, common::toJson(_service.create(_tasks.front().task_id, requiredString(_body, "filename"), _size, requiredString(_body, "target_format"))), 202); }
        catch (const common::ApiError& error) { writeError(&response, error); }
    });
    _server.Get("/api/v1/conversions/:item_id/preview", [](const httplib::Request& request, httplib::Response& response) {
        try { services::ConversionService _service; writeData(&response, common::toJson(_service.preview(requiredPathParameter(request, "item_id"))), 200); } catch (const common::ApiError& error) { writeError(&response, error); }
    });
    _server.Delete("/api/v1/conversions/:item_id", [_data_root](const httplib::Request& request, httplib::Response& response) {
        try { services::ConversionService _service; _service.remove(_data_root, requiredPathParameter(request, "item_id")); response.status = 204; } catch (const common::ApiError& error) { writeError(&response, error); }
    });
    _server.Post("/api/v1/tasks/:task_id/exports", [](const httplib::Request& request, httplib::Response& response) {
        try { services::ConversionService _service; writeData(&response, common::toJson(_service.createTaskExport(requiredPathParameter(request, "task_id"))), 202); } catch (const common::ApiError& error) { writeError(&response, error); }
    });
    _server.Get("/api/v1/exports/:export_id/download", [](const httplib::Request& request, httplib::Response& response) {
        try { services::ExportService _service; const models::ExportArchive _archive = _service.buildArchive(requiredPathParameter(request, "export_id")); response.status = 200; response.set_header("Content-Disposition", "attachment; filename=\"" + _archive.filename + "\""); response.set_content(_archive.content, "application/zip"); } catch (const common::ApiError& error) { writeError(&response, error); }
    });
    if (!_server.listen(m_configuration.listenAddress().c_str(), m_configuration.listenPort())) throw std::runtime_error("Failed to listen on configured address and port");
}

} // namespace http
} // namespace docshift
