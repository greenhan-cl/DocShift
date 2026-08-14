# DocShift Server

DocShift 的后端 HTTP 服务骨架。本阶段只提供进程配置、应用启动入口和健康检查接口，业务模块尚未实现。

## 目录

```text
apps/       可执行程序入口
cmake/      CMake 公共模块
include/    公共头文件
src/core/   进程配置与应用启动
src/health/ 健康检查
src/modules/业务模块边界说明
```

## 环境变量

| 变量 | 默认值 | 说明 |
| --- | --- | --- |
| `DOCSHIFT_SERVER_ADDRESS` | `0.0.0.0` | HTTP 监听地址。 |
| `DOCSHIFT_SERVER_PORT` | `8080` | HTTP 监听端口。 |
| `DOCSHIFT_SERVER_THREADS` | CPU 线程数 | HTTP 线程数。 |
| `DOCSHIFT_DATA_ROOT` | `./data` | 服务端数据根目录。 |

生产部署时，将服务器目录 `/home/chenlu/mycode/DocShift/data` 挂载到容器内配置的 `DOCSHIFT_DATA_ROOT`。

## 构建

构建前需要安装 C++17 编译器、CMake 和 Drogon 开发包。

```bash
cmake --preset debug
cmake --build --preset debug
```

## 健康检查

```http
GET /api/v1/health
```

该接口只验证 HTTP 进程存活，不检查数据库和转换 Worker。

