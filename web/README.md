# DocShift Web

DocShift 的前端应用骨架。本阶段只建立构建配置、应用入口、统一 API 访问和服务端健康检查，业务模块尚未实现。

## 本地启动

```bash
npm install
npm run dev
```

开发服务器默认监听 `5173`，并把 `/api` 请求代理到 `http://127.0.0.1:8080`。

## 环境变量

| 变量 | 默认值 | 说明 |
| --- | --- | --- |
| `VITE_API_BASE_URL` | `/api/v1` | 浏览器请求的 API 前缀。 |
| `DOCSHIFT_API_PROXY_TARGET` | `http://127.0.0.1:8080` | 本地开发代理的后端地址。 |

## 校验

```bash
npm run typecheck
npm run build
```

## 当前边界

- 已实现：应用外壳、响应式基础样式、健康检查 API 接入。
- 未实现：注册、登录、用户资料、Task、文件上传、转换状态、预览和导出。

