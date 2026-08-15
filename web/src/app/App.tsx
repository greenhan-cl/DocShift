import { SystemHealthCard } from "../features/system-health/SystemHealthCard";

const plannedModules = [
  {
    name: "账户与用户",
    description: "注册、邮箱验证、登录状态和用户头像。",
  },
  {
    name: "Task 工作区",
    description: "多文件拖拽、格式选择和转换状态。",
  },
  {
    name: "结果管理",
    description: "结果预览、删除和按 Task 导出。",
  },
];

export function App() {
  return (
    <div className="app-shell">
      <header className="topbar">
        <a className="brand" href="/" aria-label="DocShift 首页">
          <span className="brand-mark" aria-hidden="true">
            D
          </span>
          <span>DocShift</span>
        </a>
        <span className="stage-badge">基础架构阶段</span>
      </header>

      <main className="main-content">
        <section className="hero" aria-labelledby="page-title">
          <p className="eyebrow">Document workflow, made clear</p>
          <h1 id="page-title">文档转换工作空间</h1>
          <p className="hero-copy">
            当前前端骨架已经建立，后续会按模块逐步接入用户体系、Task、转换状态与结果管理。
          </p>
        </section>

        <section className="dashboard-grid" aria-label="系统概览">
          <SystemHealthCard />

          <article className="panel module-panel">
            <div className="panel-heading">
              <div>
                <p className="panel-label">MODULES</p>
                <h2>业务模块</h2>
              </div>
              <span className="quiet-chip">待逐步接入</span>
            </div>

            <ul className="module-list">
              {plannedModules.map((module) => (
                <li key={module.name}>
                  <span className="module-dot" aria-hidden="true" />
                  <div>
                    <h3>{module.name}</h3>
                    <p>{module.description}</p>
                  </div>
                </li>
              ))}
            </ul>
          </article>
        </section>
      </main>
    </div>
  );
}

