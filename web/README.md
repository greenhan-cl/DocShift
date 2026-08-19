# DocShift Web

DocShift 的前端应用骨架。本阶段只建立构建配置、应用入口、统一 API 访问和服务端健康检查，业务模块尚未实现。

## 本地启动

先启动 C++ 后端：

```bash
scl enable devtoolset-9 -- /home/chenlu/mycode/DocShift/server/out/build/gcc9-debug/docshift_server
```

另开一个终端启动前端；如果后端不在本机，使用 `DOCSHIFT_API_PROXY_TARGET`
指定其地址：

```bash
cd web
pnpm install
DOCSHIFT_API_PROXY_TARGET=http://36.150.116.52:18081 pnpm dev
```

开发服务器默认监听 `5173`，并把 `/api` 请求代理到 `DOCSHIFT_API_PROXY_TARGET`
指定的后端地址（默认 `http://127.0.0.1:8080`）。当前后端数据仍保存在内存中，
服务重启后会复位。

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

