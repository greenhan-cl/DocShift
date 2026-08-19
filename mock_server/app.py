"""Local in-memory API for developing the DocShift web client.

Run from this directory with: python app.py
"""

from __future__ import annotations

import json
import uuid
from copy import deepcopy
from datetime import UTC, datetime
from io import BytesIO
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Any
from urllib.parse import urlparse
from zipfile import ZIP_DEFLATED, ZipFile


def now() -> str:
    return datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z")


TASK_ID = "9a1a4c51-65f4-4a04-b81f-a6b2d84d9810"
EXPORT_EXTENSIONS = {"markdown": "md"}
DEMO_USER: dict[str, Any] = {
    "user_id": "local-demo-user",
    "username": "Chen Lu",
    "email": "demo@docshift.local",
    "avatar_url": None,
    "email_verified": True,
}
ITEMS: list[dict[str, Any]] = [
    {
        "item_id": "a72d45bc-3dfd-4bf1-bfb0-f3703a630001",
        "task_id": TASK_ID,
        "source": {"filename": "项目说明.docx", "format": "docx", "size_bytes": 245760},
        "target_format": "markdown",
        "status": "preview_ready",
        "preview_available": True,
        "download_available": True,
        "error": None,
        "created_at": "2026-08-15T10:00:00Z",
        "updated_at": "2026-08-15T10:00:08Z",
        "completed_at": "2026-08-15T10:00:08Z",
        "expires_at": None,
    },
    {
        "item_id": "a72d45bc-3dfd-4bf1-bfb0-f3703a630002",
        "task_id": TASK_ID,
        "source": {"filename": "季度汇报.pptx", "format": "pptx", "size_bytes": 8388608},
        "target_format": "markdown",
        "status": "processing",
        "preview_available": False,
        "download_available": False,
        "error": None,
        "created_at": "2026-08-15T10:01:00Z",
        "updated_at": "2026-08-15T10:01:04Z",
        "completed_at": None,
        "expires_at": None,
    },
    {
        "item_id": "a72d45bc-3dfd-4bf1-bfb0-f3703a630003",
        "task_id": TASK_ID,
        "source": {"filename": "损坏的附件.pdf", "format": "pdf", "size_bytes": 1048576},
        "target_format": "markdown",
        "status": "failed",
        "preview_available": False,
        "download_available": False,
        "error": {"code": "SOURCE_FILE_CORRUPTED", "message": "文件损坏，无法解析。"},
        "created_at": "2026-08-15T10:02:00Z",
        "updated_at": "2026-08-15T10:02:05Z",
        "completed_at": None,
        "expires_at": None,
    },
]
EXPORTS: dict[str, list[dict[str, Any]]] = {}


def response(handler: BaseHTTPRequestHandler, status: HTTPStatus, payload: dict[str, Any]) -> None:
    body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
    handler.send_response(status)
    handler.send_header("Content-Type", "application/json; charset=utf-8")
    handler.send_header("Content-Length", str(len(body)))
    handler.send_header("Access-Control-Allow-Origin", "*")
    handler.end_headers()
    handler.wfile.write(body)


def zip_response(handler: BaseHTTPRequestHandler, filename: str, items: list[dict[str, Any]]) -> None:
    archive = BytesIO()
    with ZipFile(archive, "w", ZIP_DEFLATED) as zip_file:
        for item in items:
            source_filename = str(item["source"]["filename"])
            source_stem = source_filename.rsplit(".", 1)[0]
            target_format = str(item["target_format"])
            archive_name = f"{source_stem}.{EXPORT_EXTENSIONS.get(target_format, target_format)}"
            content = (
                f"# {source_filename}\n\n"
                "这是由 DocShift 本地 mock 服务生成的演示导出内容。\n\n"
                f"- 源格式：{item['source']['format']}\n"
                f"- 目标格式：{target_format}\n"
                "- 转换状态：已完成\n"
            )
            zip_file.writestr(archive_name, content)

    body = archive.getvalue()
    handler.send_response(HTTPStatus.OK)
    handler.send_header("Content-Type", "application/zip")
    handler.send_header("Content-Disposition", f'attachment; filename="{filename}"')
    handler.send_header("Content-Length", str(len(body)))
    handler.send_header("Access-Control-Allow-Origin", "*")
    handler.end_headers()
    handler.wfile.write(body)


def task_summary() -> dict[str, Any]:
    active_items = [item for item in ITEMS if item["status"] != "deleted"]
    statuses = [item["status"] for item in active_items]
    return {
        "task_id": TASK_ID,
        "display_name": "本地演示任务",
        "counts": {
            "total": len(active_items),
            "processing": sum(status in {"queued", "processing"} for status in statuses),
            "preview_ready": statuses.count("preview_ready"),
            "failed": statuses.count("failed"),
            "expired": statuses.count("expired"),
        },
        "created_at": "2026-08-15T10:00:00Z",
        "updated_at": now(),
        "expires_at": None,
    }


def conversion_items() -> list[dict[str, Any]]:
    return [item for item in ITEMS if item["status"] != "deleted"]


class ApiHandler(BaseHTTPRequestHandler):
    server_version = "DocShiftMock/0.1"

    def log_message(self, format: str, *args: object) -> None:
        print(f"[{self.log_date_time_string()}] {format % args}")

    def do_OPTIONS(self) -> None:  # noqa: N802
        self.send_response(HTTPStatus.NO_CONTENT)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, PATCH, POST, DELETE, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type, Idempotency-Key")
        self.end_headers()

    def do_GET(self) -> None:  # noqa: N802
        path = urlparse(self.path).path
        if path == "/api/v1/health":
            response(self, HTTPStatus.OK, {"service": "docshift-mock-server", "status": "ok", "version": "0.1.0"})
        elif path == "/api/v1/conversions":
            response(self, HTTPStatus.OK, {"data": {"items": conversion_items()}, "request_id": str(uuid.uuid4())})
        elif path.startswith("/api/v1/conversions/") and path.endswith("/preview"):
            item_id = path.removeprefix("/api/v1/conversions/").removesuffix("/preview")
            item = next((candidate for candidate in ITEMS if candidate["item_id"] == item_id and candidate["status"] != "deleted"), None)
            if item is None:
                response(self, HTTPStatus.NOT_FOUND, {"error": {"code": "CONVERSION_NOT_FOUND", "message": "转换文件不存在"}})
            elif item["status"] == "preview_ready":
                response(self, HTTPStatus.OK, {"data": {"title": f"{item['source']['filename']} · 转换结果", "kind": "result", "content": f"# {item['source']['filename']}\n\n这是由本地 mock 服务生成的 **{item['target_format'].upper()}** 预览内容。\n\n- 源格式：{item['source']['format'].upper()}\n- 转换状态：已完成\n- 可在此处确认结果后导出。"}, "request_id": str(uuid.uuid4())})
            else:
                response(self, HTTPStatus.OK, {"data": {"title": item["source"]["filename"], "kind": "source", "content": f"源文件预览\n\n文件名：{item['source']['filename']}\n格式：{item['source']['format'].upper()}\n大小：{item['source']['size_bytes']} bytes\n\n当前状态：{item['status']}。转换完成后将在此显示结果预览。"}, "request_id": str(uuid.uuid4())})
        elif path.startswith("/api/v1/exports/") and path.endswith("/download"):
            export_id = path.removeprefix("/api/v1/exports/").removesuffix("/download")
            items = EXPORTS.get(export_id)
            if items is None:
                response(self, HTTPStatus.NOT_FOUND, {"error": {"code": "EXPORT_NOT_FOUND", "message": "导出记录不存在或已过期。"}})
            else:
                zip_response(self, "docshift-export.zip", items)
        elif path == "/api/v1/tasks":
            response(self, HTTPStatus.OK, {"data": {"items": [task_summary()], "next_cursor": None}, "request_id": str(uuid.uuid4())})
        elif path == f"/api/v1/tasks/{TASK_ID}":
            response(self, HTTPStatus.OK, {"data": {"task": task_summary(), "items": [item for item in ITEMS if item["status"] != "deleted"]}, "request_id": str(uuid.uuid4())})
        else:
            response(self, HTTPStatus.NOT_FOUND, {"error": {"code": "NOT_FOUND", "message": "Mock endpoint not found"}})

    def do_PATCH(self) -> None:  # noqa: N802
        path = urlparse(self.path).path
        if path != "/api/v1/users/me":
            response(self, HTTPStatus.NOT_FOUND, {"error": {"code": "NOT_FOUND", "message": "Mock endpoint not found"}})
            return

        length = int(self.headers.get("Content-Length", "0"))
        try:
            payload = json.loads(self.rfile.read(length) or b"{}")
        except json.JSONDecodeError:
            payload = {}

        username = str(payload.get("username", "")).strip()
        avatar_url = payload.get("avatar_url")
        if not username:
            response(self, HTTPStatus.UNPROCESSABLE_ENTITY, {"error": {"code": "USERNAME_INVALID", "message": "昵称不能为空。"}})
            return
        if len(username) > 32:
            response(self, HTTPStatus.UNPROCESSABLE_ENTITY, {"error": {"code": "USERNAME_TOO_LONG", "message": "昵称不能超过 32 个字符。"}})
            return
        if avatar_url is not None and not isinstance(avatar_url, str):
            response(self, HTTPStatus.UNPROCESSABLE_ENTITY, {"error": {"code": "AVATAR_INVALID", "message": "头像数据格式不正确。"}})
            return
        if isinstance(avatar_url, str) and len(avatar_url) > 1_500_000:
            response(self, HTTPStatus.UNPROCESSABLE_ENTITY, {"error": {"code": "AVATAR_TOO_LARGE", "message": "演示头像数据过大。"}})
            return

        DEMO_USER["username"] = username
        DEMO_USER["avatar_url"] = avatar_url
        response(self, HTTPStatus.OK, {"data": {"user": DEMO_USER}, "request_id": str(uuid.uuid4())})

    def do_POST(self) -> None:  # noqa: N802
        path = urlparse(self.path).path
        if path == "/api/v1/auth/email-verifications":
            length = int(self.headers.get("Content-Length", "0"))
            try:
                payload = json.loads(self.rfile.read(length) or b"{}")
            except json.JSONDecodeError:
                payload = {}
            email = str(payload.get("email", ""))
            if "@" not in email:
                response(self, HTTPStatus.UNPROCESSABLE_ENTITY, {"error": {"code": "EMAIL_INVALID", "message": "请输入有效的邮箱地址。"}})
                return
            response(self, HTTPStatus.ACCEPTED, {"data": {"verification_id": str(uuid.uuid4()), "expires_at": "2026-08-15T12:00:00Z", "retry_after_seconds": 60, "demo_code": "123456"}, "request_id": str(uuid.uuid4())})
            return
        if path == "/api/v1/conversions":
            length = int(self.headers.get("Content-Length", "0"))
            try:
                payload = json.loads(self.rfile.read(length) or b"{}")
            except json.JSONDecodeError:
                payload = {}
            filename = str(payload.get("filename", "未命名文件"))
            suffix = filename.rsplit(".", 1)[-1].lower() if "." in filename else "file"
            item = {"item_id": str(uuid.uuid4()), "task_id": None, "source": {"filename": filename, "format": suffix, "size_bytes": int(payload.get("size_bytes", 0))}, "target_format": str(payload.get("target_format", "markdown")), "status": "processing", "preview_available": False, "download_available": False, "error": None, "created_at": now(), "updated_at": now(), "completed_at": None, "expires_at": None}
            ITEMS.insert(0, item)
            response(self, HTTPStatus.ACCEPTED, {"data": item, "request_id": str(uuid.uuid4())})
            return
        if path == "/api/v1/auth/login":
            length = int(self.headers.get("Content-Length", "0"))
            try:
                payload = json.loads(self.rfile.read(length) or b"{}")
            except json.JSONDecodeError:
                payload = {}
            if payload.get("verification_code") != "123456":
                response(self, HTTPStatus.UNAUTHORIZED, {"error": {"code": "VERIFICATION_CODE_INVALID", "message": "验证码不正确。"}})
                return
            DEMO_USER["email"] = str(payload.get("email", "demo@docshift.local"))
            response(self, HTTPStatus.OK, {"data": {"user": DEMO_USER}, "request_id": str(uuid.uuid4())})
            return
        if path == "/api/v1/tasks":
            response(self, HTTPStatus.CREATED, {"data": task_summary(), "request_id": str(uuid.uuid4())})
            return
        if path == f"/api/v1/tasks/{TASK_ID}/items":
            length = int(self.headers.get("Content-Length", "0"))
            try:
                payload = json.loads(self.rfile.read(length) or b"{}")
            except json.JSONDecodeError:
                payload = {}
            filename = str(payload.get("filename", "未命名文件.md"))
            target_format = str(payload.get("target_format", "markdown"))
            suffix = filename.rsplit(".", 1)[-1].lower() if "." in filename else "file"
            item = {
                "item_id": str(uuid.uuid4()), "task_id": TASK_ID,
                "source": {"filename": filename, "format": suffix, "size_bytes": int(payload.get("size_bytes", 524288))},
                "target_format": target_format, "status": "preview_ready",
                "preview_available": True, "download_available": True, "error": None,
                "created_at": now(), "updated_at": now(), "completed_at": now(), "expires_at": None,
            }
            ITEMS.insert(0, item)
            response(self, HTTPStatus.ACCEPTED, {"data": item, "request_id": str(uuid.uuid4())})
            return
        if path == f"/api/v1/tasks/{TASK_ID}/exports":
            ready_items = [item for item in ITEMS if item["task_id"] == TASK_ID and item["download_available"] and item["status"] != "deleted"]
            if not ready_items:
                response(self, HTTPStatus.CONFLICT, {"error": {"code": "NO_EXPORTABLE_ITEMS", "message": "当前任务没有可导出的文件。"}})
                return
            export_id = str(uuid.uuid4())
            EXPORTS[export_id] = deepcopy(ready_items)
            response(self, HTTPStatus.ACCEPTED, {"data": {"export_id": export_id, "filename": "docshift-export.zip", "item_count": len(ready_items), "download_url": f"/exports/{export_id}/download"}, "request_id": str(uuid.uuid4())})
            return
        response(self, HTTPStatus.NOT_FOUND, {"error": {"code": "NOT_FOUND", "message": "Mock endpoint not found"}})

    def do_DELETE(self) -> None:  # noqa: N802
        prefix = "/api/v1/items/"
        suffix = "/result"
        path = urlparse(self.path).path
        if path.startswith(prefix) and path.endswith(suffix):
            item_id = path[len(prefix):-len(suffix)]
            for item in ITEMS:
                if item["item_id"] == item_id:
                    item["status"] = "deleted"
                    item["download_available"] = False
                    item["preview_available"] = False
                    response(self, HTTPStatus.NO_CONTENT, {})
                    return
        conversion_prefix = "/api/v1/conversions/"
        if path.startswith(conversion_prefix):
            item_id = path.removeprefix(conversion_prefix)
            for item in ITEMS:
                if item["item_id"] == item_id:
                    item["status"] = "deleted"
                    item["download_available"] = False
                    item["preview_available"] = False
                    response(self, HTTPStatus.NO_CONTENT, {})
                    return
        response(self, HTTPStatus.NOT_FOUND, {"error": {"code": "ITEM_NOT_FOUND", "message": "转换项不存在"}})


if __name__ == "__main__":
    print("DocShift mock server listening on http://127.0.0.1:8080")
    ThreadingHTTPServer(("127.0.0.1", 8080), ApiHandler).serve_forever()
