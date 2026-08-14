# DocShift API 设计

## 1. 文档范围

本文定义前端与后端之间的公开 HTTP API，包括资源边界、请求响应结构、鉴权、幂等、错误码和状态推送。Worker 通过后端任务调度机制领取任务，不提供公开 API。

当前尚未确认注册后的登录方式、Task 名称和导出交付方式。本文对已确认接口给出完整草案，对受这些决策影响的部分明确标注待确认。

## 2. 通用约定

### 2.1 基础规则

- API 前缀：`/api/v1`。
- 普通请求和响应使用 `application/json; charset=utf-8`。
- 文件上传使用 `multipart/form-data`。
- 文件下载根据文件类型返回实际 `Content-Type`，并设置安全的 `Content-Disposition`。
- 时间使用 UTC ISO 8601，例如 `2026-08-14T09:30:00Z`。
- 资源标识使用 UUID 字符串。
- 前端不得传入或获得服务器绝对文件路径。
- 未识别的请求字段默认拒绝，避免拼写错误被静默忽略。

### 2.2 请求头

| 请求头 | 必填 | 说明 |
| --- | --- | --- |
| `X-Request-ID` | 否 | 调用方可提供；未提供时由服务端生成。 |
| `Idempotency-Key` | 按接口 | 创建 Task、转换项和导出请求时推荐必填。 |
| `X-CSRF-Token` | 登录后写操作 | Cookie 登录态下的跨站请求保护令牌。 |
| `Accept-Language` | 否 | 用于用户错误文案语言；业务错误码保持稳定。 |

### 2.3 成功响应

普通成功响应：

```json
{
  "data": {},
  "request_id": "01J..."
}
```

列表响应：

```json
{
  "data": {
    "items": [],
    "next_cursor": null
  },
  "request_id": "01J..."
}
```

### 2.4 失败响应

```json
{
  "error": {
    "code": "TASK_NOT_FOUND",
    "message": "Task 不存在或无权访问",
    "details": []
  },
  "request_id": "01J..."
}
```

安全要求：对于无权访问和资源不存在，统一返回相同状态码和错误码，避免泄露其他用户的资源是否存在。

### 2.5 分页

- 列表接口使用游标分页，不使用页码分页。
- 通用参数：`limit`、`cursor`。
- `limit` 的默认值和最大值待性能设计时确认。
- 游标由服务端生成，客户端不得解析或修改。

## 3. 鉴权与权限

### 3.1 登录态

- 用户完成身份鉴别后，服务端创建服务器会话，并通过安全 Cookie 传递不透明会话令牌。
- Cookie 必须使用 `HttpOnly`、`Secure` 和合适的 `SameSite` 属性。
- 数据库只保存会话令牌摘要。
- 除注册、验证码和后续登录接口外，其余接口均要求有效登录态。
- 注册后的具体登录接口取决于“密码登录”还是“每次邮箱验证码登录”，当前暂不定义请求字段。

### 3.2 所有权校验

每个 Task 相关请求都必须按以下顺序校验：

```text
登录态 → 当前 user_id → Task.user_id → ConversionItem.task_id → FileAsset 所属关系
```

不能仅凭 `task_id`、`item_id` 或 `asset_id` 授权。

### 3.3 CSRF 与来源校验

- 登录后所有 `POST`、`PUT`、`PATCH`、`DELETE` 请求必须校验 CSRF 令牌。
- 服务端同时校验 `Origin` 或 `Referer` 是否来自允许站点。
- 上传、删除和导出接口不能使用 GET 触发状态变化。

## 4. 公共数据结构

### 4.1 User

```json
{
  "user_id": "uuid",
  "username": "alice",
  "email": "alice@example.com",
  "email_verified": true,
  "avatar_url": "/api/v1/users/me/avatar",
  "created_at": "2026-08-14T09:30:00Z",
  "updated_at": "2026-08-14T09:30:00Z"
}
```

`avatar_url` 是受控 API 地址，不是服务器文件路径。没有头像时为 `null`。

### 4.2 TaskSummary

```json
{
  "task_id": "uuid",
  "display_name": null,
  "counts": {
    "total": 3,
    "processing": 1,
    "preview_ready": 1,
    "failed": 1,
    "expired": 0
  },
  "created_at": "2026-08-14T09:30:00Z",
  "updated_at": "2026-08-14T09:35:00Z",
  "expires_at": null
}
```

### 4.3 ConversionItem

```json
{
  "item_id": "uuid",
  "task_id": "uuid",
  "source": {
    "filename": "example.docx",
    "format": "docx",
    "size_bytes": 102400
  },
  "target_format": "markdown",
  "status": "preview_ready",
  "preview_available": true,
  "download_available": true,
  "error": null,
  "created_at": "2026-08-14T09:31:00Z",
  "updated_at": "2026-08-14T09:32:00Z",
  "completed_at": "2026-08-14T09:32:00Z",
  "expires_at": null
}
```

允许状态：

```text
uploading
queued
processing
preview_ready
failed
deleted
expired
```

失败时：

```json
{
  "code": "SOURCE_FILE_CORRUPTED",
  "message": "文件损坏，无法解析"
}
```

不返回内部命令、堆栈、临时目录或转换工具原始日志。

## 5. 用户与身份 API

### 5.1 请求注册邮箱验证码

```http
POST /api/v1/auth/email-verifications
Content-Type: application/json
```

请求：

```json
{
  "email": "alice@example.com",
  "purpose": "registration"
}
```

响应：`202 Accepted`

```json
{
  "data": {
    "verification_id": "uuid",
    "expires_at": "2026-08-14T09:40:00Z",
    "retry_after_seconds": 60
  },
  "request_id": "01J..."
}
```

规则：

- 验证码有效期、重新发送间隔、单邮箱和单来源频率上限待确认。
- 验证码原文只通过邮件发送，不出现在响应或日志中。
- 频率超限返回 `429 Too Many Requests` 和 `EMAIL_VERIFICATION_RATE_LIMITED`。
- 邮件服务临时失败返回 `503 Service Unavailable`，不创建可使用的验证码记录。

### 5.2 完成注册

```http
POST /api/v1/auth/register
Content-Type: application/json
```

请求：

```json
{
  "username": "alice",
  "email": "alice@example.com",
  "verification_id": "uuid",
  "verification_code": "123456"
}
```

响应：`201 Created`

```json
{
  "data": {
    "user": {
      "user_id": "uuid",
      "username": "alice",
      "email": "alice@example.com",
      "email_verified": true,
      "avatar_url": null,
      "created_at": "2026-08-14T09:30:00Z",
      "updated_at": "2026-08-14T09:30:00Z"
    }
  },
  "request_id": "01J..."
}
```

规则：

- 用户名、邮箱和验证码校验在同一注册事务中完成。
- 验证码只能使用一次。
- 用户名重复返回 `USERNAME_ALREADY_EXISTS`。
- 邮箱重复返回 `EMAIL_ALREADY_EXISTS`。
- 注册成功后是否立即创建登录会话，待登录方式确认后确定。

### 5.3 登录接口（待确认）

保留资源：

```http
POST /api/v1/auth/login
```

在确定以下方案前，不定义请求体：

- 用户名/邮箱 + 密码；或
- 用户名/邮箱 + 每次邮箱验证码。

### 5.4 退出登录

```http
DELETE /api/v1/auth/session
```

响应：`204 No Content`。接口吊销当前会话；重复调用保持幂等。

## 6. 用户资料 API

### 6.1 查询当前用户

```http
GET /api/v1/users/me
```

响应：`200 OK`，返回 `User`。

### 6.2 修改当前用户资料

```http
PATCH /api/v1/users/me
Content-Type: application/json
```

请求：

```json
{
  "username": "alice-new"
}
```

响应：`200 OK`，返回更新后的 `User`。

MVP 暂不允许通过此接口直接修改邮箱；修改邮箱需要新的邮箱验证流程，需求确认后另行设计。

### 6.3 上传或替换头像

```http
PUT /api/v1/users/me/avatar
Content-Type: multipart/form-data
```

表单字段：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| `file` | binary | 是 | 头像图片。 |

响应：`200 OK`

```json
{
  "data": {
    "avatar_url": "/api/v1/users/me/avatar",
    "updated_at": "2026-08-14T09:40:00Z"
  },
  "request_id": "01J..."
}
```

规则：

- 先完整保存并校验新头像，再切换用户头像引用。
- 上传失败不影响旧头像。
- 图片格式、尺寸和文件大小上限待确认。

### 6.4 获取当前头像

```http
GET /api/v1/users/me/avatar
```

响应为图片二进制。未设置头像返回 `404 AVATAR_NOT_FOUND`。

## 7. Task API

### 7.1 创建 Task

```http
POST /api/v1/tasks
Idempotency-Key: <unique-key>
Content-Type: application/json
```

请求：

```json
{
  "display_name": null
}
```

`display_name` 是否由用户填写待确认；确认前允许为空。

响应：`201 Created`，返回 `TaskSummary`。

### 7.2 查询 Task 列表

```http
GET /api/v1/tasks?limit=20&cursor=<opaque-cursor>
```

响应：`200 OK`

```json
{
  "data": {
    "items": [],
    "next_cursor": null
  },
  "request_id": "01J..."
}
```

只返回当前用户的 Task，按最近更新时间倒序排列。

### 7.3 查询 Task 详情

```http
GET /api/v1/tasks/{task_id}
```

响应：`200 OK`

```json
{
  "data": {
    "task": {},
    "items": []
  },
  "request_id": "01J..."
}
```

当转换项较多时，`items` 应改为独立游标分页；单 Task 文件数量上限确认后决定是否首期拆分。

MVP 当前不定义删除 Task 接口。

## 8. 转换项 API

### 8.1 上传文件并创建转换项

前端支持一次选择多个文件，但 API 每次创建一个转换项，便于逐文件显示上传进度、失败和重试。

```http
POST /api/v1/tasks/{task_id}/items
Idempotency-Key: <unique-key>
Content-Type: multipart/form-data
```

表单字段：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| `file` | binary | 是 | 一个源文件。 |
| `target_format` | string | 是 | 目标格式。 |
| `conversion_options` | JSON string | 否 | 受白名单约束的转换设置，默认 `{}`。 |

响应：`202 Accepted`，返回 `ConversionItem`。

规则：

- 服务端重新识别源格式，不能只信任扩展名或前端检测结果。
- 不支持的转换组合返回 `UNSUPPORTED_CONVERSION_ROUTE`。
- 文件校验通过后才创建可执行任务。
- 达到文件大小或 Task 数量上限返回 `UPLOAD_LIMIT_EXCEEDED`。
- 相同 `Idempotency-Key` 和相同请求应返回同一个转换项；请求内容不同返回 `IDEMPOTENCY_KEY_CONFLICT`。

### 8.2 查询转换项

```http
GET /api/v1/items/{item_id}
```

响应：`200 OK`，返回 `ConversionItem`。

### 8.3 删除已完成结果

```http
DELETE /api/v1/items/{item_id}/result
```

响应：`204 No Content`。

规则：

- 只允许删除 `preview_ready` 的结果。
- 删除后结果和预览立即不能访问，物理文件异步清理。
- 对已经删除的结果重复请求返回 `204`，保持幂等。
- 是否同时删除源文件待确认。

## 9. 预览 API

### 9.1 查询预览清单

```http
GET /api/v1/items/{item_id}/preview
```

响应：`200 OK`

```json
{
  "data": {
    "item_id": "uuid",
    "previews": [
      {
        "preview_id": "uuid",
        "type": "markdown",
        "content_url": "/api/v1/items/uuid/previews/uuid"
      }
    ]
  },
  "request_id": "01J..."
}
```

预览类型示例：`markdown`、`html`、`table_json`。

### 9.2 获取预览内容

```http
GET /api/v1/items/{item_id}/previews/{preview_id}
```

响应根据类型返回：

- Markdown：`text/markdown; charset=utf-8`。
- 隔离 HTML：`text/html; charset=utf-8`，带严格安全响应头。
- XLSX 表格预览：`application/json`。

规则：

- 每次请求都校验当前用户、Task、转换项和预览资产所有权。
- 已删除、失败或过期结果返回 `410 Gone` 和 `RESULT_NOT_AVAILABLE`。
- HTML 预览禁止以主站权限执行脚本。

## 10. 导出 API

### 10.1 创建导出请求

```http
POST /api/v1/tasks/{task_id}/exports
Idempotency-Key: <unique-key>
Content-Type: application/json
```

导出选中文件：

```json
{
  "scope": "selected",
  "item_ids": ["uuid-1", "uuid-2"]
}
```

导出所有可用文件：

```json
{
  "scope": "all_available"
}
```

响应：`202 Accepted`

```json
{
  "data": {
    "export_id": "uuid",
    "state": "preparing",
    "delivery_mode": "archive"
  },
  "request_id": "01J..."
}
```

规则：

- 所有指定转换项必须属于当前用户和当前 Task。
- 只允许导出 `preview_ready` 且文件可访问的结果。
- 创建导出请求时冻结结果资产集合；如果其中某个结果随后被用户删除，包含该结果的导出请求立即失效，必须重新创建导出。
- `delivery_mode` 最终取 `direct` 还是 `archive`，取决于待确认的产品策略。

### 10.2 查询导出状态

```http
GET /api/v1/exports/{export_id}
```

响应：`200 OK`

```json
{
  "data": {
    "export_id": "uuid",
    "task_id": "uuid",
    "state": "ready",
    "delivery_mode": "archive",
    "files": [],
    "download_url": "/api/v1/exports/uuid/download",
    "expires_at": "2026-08-15T09:30:00Z"
  },
  "request_id": "01J..."
}
```

- 压缩包模式使用 `download_url`。
- 逐文件模式使用 `files` 返回受控下载地址，`download_url` 为 `null`。
- 导出状态为 `invalidated` 时，表示其中至少一个结果已被删除，原下载地址不再可用。

### 10.3 下载导出结果

```http
GET /api/v1/exports/{export_id}/download
```

响应为文件二进制。仅当导出状态为 `ready` 且当前用户拥有该 Task 时允许下载。

## 11. 状态事件 API

### 11.1 建立事件流

```http
GET /api/v1/events
Accept: text/event-stream
Last-Event-ID: <event-id>
```

事件示例：

```text
id: 01J...
event: conversion_item.updated
data: {"task_id":"uuid","item_id":"uuid","status":"processing","updated_at":"2026-08-14T09:31:30Z"}
```

Task 汇总变化：

```text
event: task.summary_updated
data: {"task_id":"uuid","counts":{"total":3,"processing":1,"preview_ready":1,"failed":1}}
```

规则：

- 只推送当前用户拥有的 Task 事件。
- 服务端定期发送心跳，避免代理误判连接空闲。
- 断线后前端使用 `Last-Event-ID` 尝试续传；无法续传时重新查询 Task 详情。
- 事件流不是唯一数据源，页面刷新后仍应通过查询 API 恢复完整状态。

## 12. HTTP 状态码

| 状态码 | 使用场景 |
| --- | --- |
| `200 OK` | 查询或更新成功。 |
| `201 Created` | 用户、Task 等同步创建成功。 |
| `202 Accepted` | 邮件发送、转换或导出已接受异步处理。 |
| `204 No Content` | 退出或删除成功。 |
| `400 Bad Request` | JSON、表单或参数格式错误。 |
| `401 Unauthorized` | 未登录或会话失效。 |
| `403 Forbidden` | 已认证但操作被策略禁止；资源所有权错误仍使用 404。 |
| `404 Not Found` | 资源不存在或当前用户无权访问。 |
| `409 Conflict` | 用户名/邮箱重复、状态冲突、幂等键冲突。 |
| `410 Gone` | 结果已删除或已过期。 |
| `413 Content Too Large` | 上传文件超过限制。 |
| `415 Unsupported Media Type` | 文件类型或请求媒体类型不支持。 |
| `422 Unprocessable Content` | 参数语法正确但转换组合或业务规则不满足。 |
| `429 Too Many Requests` | 验证码发送或接口频率超限。 |
| `500 Internal Server Error` | 未分类服务端错误。 |
| `503 Service Unavailable` | 邮件、文件系统或转换服务暂时不可用。 |

## 13. 业务错误码

### 13.1 用户与身份

| 错误码 | 含义 |
| --- | --- |
| `AUTH_REQUIRED` | 需要登录。 |
| `SESSION_EXPIRED` | 会话已过期。 |
| `CSRF_VALIDATION_FAILED` | CSRF 校验失败。 |
| `USERNAME_ALREADY_EXISTS` | 用户名已存在。 |
| `EMAIL_ALREADY_EXISTS` | 邮箱已注册。 |
| `EMAIL_VERIFICATION_INVALID` | 验证码错误。 |
| `EMAIL_VERIFICATION_EXPIRED` | 验证码已过期。 |
| `EMAIL_VERIFICATION_RATE_LIMITED` | 验证码请求过于频繁。 |
| `EMAIL_DELIVERY_FAILED` | 邮件暂时发送失败。 |
| `AVATAR_NOT_FOUND` | 用户未设置头像。 |
| `AVATAR_INVALID` | 头像类型、尺寸或内容不合法。 |

### 13.2 Task 与转换

| 错误码 | 含义 |
| --- | --- |
| `TASK_NOT_FOUND` | Task 不存在或无权访问。 |
| `ITEM_NOT_FOUND` | 转换项不存在或无权访问。 |
| `UNSUPPORTED_SOURCE_FORMAT` | 源文件格式不支持。 |
| `UNSUPPORTED_CONVERSION_ROUTE` | 源格式到目标格式的组合不支持。 |
| `SOURCE_FILE_CORRUPTED` | 源文件损坏。 |
| `SOURCE_FILE_PASSWORD_PROTECTED` | 文件受密码保护。 |
| `UPLOAD_LIMIT_EXCEEDED` | 文件大小或 Task 文件数量超过限制。 |
| `CONVERSION_FAILED` | 转换失败。 |
| `CONVERSION_TIMEOUT` | 转换超时。 |
| `ITEM_STATE_CONFLICT` | 当前状态不允许执行该操作。 |
| `RESULT_NOT_AVAILABLE` | 结果未生成、已删除或已过期。 |
| `EXPORT_EMPTY` | 当前没有可导出的结果。 |
| `EXPORT_FAILED` | 导出准备失败。 |
| `IDEMPOTENCY_KEY_CONFLICT` | 同一幂等键对应了不同请求。 |

## 14. 幂等、并发与重试

- `POST /tasks`、`POST /tasks/{id}/items`、`POST /tasks/{id}/exports` 使用 `Idempotency-Key` 防止前端重试造成重复资源。
- 幂等记录至少绑定当前用户、接口、请求摘要和响应资源标识。
- 删除结果和退出登录为天然幂等操作。
- 前端只对网络中断、超时和明确可重试的 `503` 自动重试；不能自动重试业务校验失败。
- 查询接口可安全重试。
- 转换项状态更新使用服务端乐观锁或条件更新，防止 Worker 完成与用户删除互相覆盖。

## 15. 上传与下载安全

- 上传时同时校验扩展名、媒体类型和文件内容特征。
- 用户文件名只用于展示，实际路径和文件名由服务端生成。
- 所有下载响应校验用户、Task、转换项和文件资产关系。
- 下载文件名移除换行、路径分隔符和其他危险字符。
- HTML 预览使用严格内容安全策略，并与主站执行上下文隔离。
- API 日志不记录上传内容、验证码、会话令牌和服务器绝对路径。

## 16. 待确认事项

1. 注册后的登录方式及 `POST /auth/login` 的最终请求结构。
2. 注册成功后是否自动创建登录会话。
3. 用户名、验证码、会话、文件和接口限流的具体数值。
4. Task 是否使用用户自定义名称。
5. Task 详情中的转换项是否从首期开始独立分页。
6. 导出采用逐文件下载还是生成压缩包。
7. 头像格式、尺寸和大小限制。
8. 文件删除和保留策略。
