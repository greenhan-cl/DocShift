# DocShift Web

DocShift 的前端应用骨架。本阶段只建立构建配置、应用入口、统一 API 访问和服务端健康检查，业务模块尚未实现。

## 本地启动

先启动假后端（无需安装 Python 包）：

```bash
cd ../mock_server
python app.py
```

另开一个终端启动前端：

```bash
pnpm install
pnpm dev
```

开发服务器默认监听 `5173`，并把 `/api` 请求代理到 `http://127.0.0.1:8080`。
打开 `http://127.0.0.1:5173` 后，可查看预置的转换任务、添加本地文件进行模拟转换、删除结果并模拟导出。Mock 服务只在内存中保存数据，重启后会复位。

## 环境变量

| 变量 | 默认值 | 说明 |
| --- | --- | --- |
| `VITE_API_BASE_URL` | `/api/v1` | 浏览器请求的 API 前缀。 |
| `DOCSHIFT_API_PROXY_TARGET` | `http://127.0.0.1:8080` | 本地开发代理的后端地址。 |

## 校验

```bash
pnpm typecheck
pnpm build
```

## 当前边界

- 已实现：应用外壳、响应式基础样式、健康检查 API 接入。
- 未实现：注册、登录、用户资料、Task、文件上传、转换状态、预览和导出。

