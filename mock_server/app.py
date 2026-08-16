"""Local in-memory API for developing the DocShift web client.

Run from this directory with: python app.py
"""

from __future__ import annotations

import json
import uuid
from datetime import UTC, datetime
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Any
from urllib.parse import urlparse


def now() -> str:
    return datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z")


TASK_ID = "9a1a4c51-65f4-4a04-b81f-a6b2d84d9810"
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


def response(handler: BaseHTTPRequestHandler, status: HTTPStatus, payload: dict[str, Any]) -> None:
    body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
    handler.send_response(status)
    handler.send_header("Content-Type", "application/json; charset=utf-8")
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
        self.send_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS")
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
        elif path == "/api/v1/tasks":
            response(self, HTTPStatus.OK, {"data": {"items": [task_summary()], "next_cursor": None}, "request_id": str(uuid.uuid4())})
        elif path == f"/api/v1/tasks/{TASK_ID}":
            response(self, HTTPStatus.OK, {"data": {"task": task_summary(), "items": [item for item in ITEMS if item["status"] != "deleted"]}, "request_id": str(uuid.uuid4())})
        else:
            response(self, HTTPStatus.NOT_FOUND, {"error": {"code": "NOT_FOUND", "message": "Mock endpoint not found"}})

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
            response(self, HTTPStatus.OK, {"data": {"user": {"user_id": "local-demo-user", "username": "Chen Lu", "email": str(payload.get("email", "demo@docshift.local")), "email_verified": True}}, "request_id": str(uuid.uuid4())})
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
            ready_items = [item for item in ITEMS if item["download_available"] and item["status"] != "deleted"]
            response(self, HTTPStatus.ACCEPTED, {"data": {"export_id": str(uuid.uuid4()), "filename": "docshift-export.zip", "item_count": len(ready_items)}, "request_id": str(uuid.uuid4())})
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
