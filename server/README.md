# DocShift Server

DocShift 的后端 HTTP 服务。当前阶段提供与 React 前端匹配的内存演示 API；服务进程重启后，用户资料、转换记录和导出记录会恢复为演示初始数据。

## 目录

```text
cmake/      CMake 公共模块
include/    项目头文件，按模块与 src/ 对应
src/main.cc 可执行程序入口
src/http/   HTTP 服务启动
src/handlers/HTTP 请求处理器
src/config/ 进程配置
src/services/业务服务
src/storage/ 文件与持久化存储
src/models/  领域数据模型
src/common/  公共基础设施
tests/      自动化测试
```

## 当前演示范围

- 邮箱验证码固定为 `123456`，不会发送真实邮件。
- `POST /api/v1/tasks/{task_id}/items` 接收单个 `multipart/form-data` 文件，并先保存到本地 Task 目录；转换仍为内存演示状态，不会调用 Pandoc 或 MarkItDown。
- 导出接口会生成包含演示文本的 ZIP 文件。
- 当前数据仅用于前端联调；PostgreSQL、文件系统、SMTP 和转换引擎将在后续阶段接入。

## 环境变量

| 变量 | 默认值 | 说明 |
| --- | --- | --- |
| `DOCSHIFT_SERVER_ADDRESS` | `0.0.0.0` | HTTP 监听地址。 |
| `DOCSHIFT_SERVER_PORT` | `8080` | HTTP 监听端口。 |
| `DOCSHIFT_SERVER_THREADS` | CPU 线程数 | HTTP 线程数。 |
| `DOCSHIFT_DATA_ROOT` | `./data` | 服务端数据根目录。 |
| `DOCSHIFT_UPLOAD_MAX_BYTES` | `52428800` | 单文件上传上限，默认 50 MiB。 |

生产部署时，将服务器目录 `/home/chenlu/mycode/DocShift/data` 挂载到容器内配置的 `DOCSHIFT_DATA_ROOT`。上传文件按 `users/{邮箱派生目录}/tasks/{task_id}/sources/` 暂存；接入 PostgreSQL 后会改为稳定 `user_id` 目录。

## 构建

构建前需要安装支持 C++11 的编译器和 CMake 3.16 以上版本。HTTP 与 JSON
库以单头文件形式随项目源码提供，无需在系统中安装 Drogon、cpp-httplib 或
nlohmann/json。

```bash
cmake3 --preset debug
cmake --build --preset debug
```

在使用 `cmake3` 命令的系统上，第二条命令也应写为 `cmake3 --build --preset debug`。

## 健康检查

```http
GET /api/v1/health
```

该接口只验证 HTTP 进程存活，不检查数据库和转换 Worker。

## 前端联调接口

```text
POST   /api/v1/auth/email-verifications
POST   /api/v1/auth/login
PATCH  /api/v1/users/me
GET    /api/v1/tasks
POST   /api/v1/tasks
GET    /api/v1/tasks/{task_id}
POST   /api/v1/tasks/{task_id}/items
GET    /api/v1/conversions
POST   /api/v1/conversions
GET    /api/v1/conversions/{item_id}/preview
DELETE /api/v1/conversions/{item_id}
POST   /api/v1/tasks/{task_id}/exports
GET    /api/v1/exports/{export_id}/download
```
