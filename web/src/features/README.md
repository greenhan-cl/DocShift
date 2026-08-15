# 前端业务模块

业务功能将在后续阶段逐个落地，本轮仅实现服务端健康检查模块。

规划目录：

```text
features/
  auth/               注册、邮箱验证和登录
  profile/            用户资料与头像
  tasks/              Task 列表与创建
  task-workspace/     文件拖拽、上传与格式配置
  conversion-status/ 转换状态与事件更新
  results/            预览、删除与导出
  system-health/      基础服务连通性
```

业务模块只能通过 `shared/api` 访问后端，不能直接保存服务器文件路径或绕过统一错误处理。

