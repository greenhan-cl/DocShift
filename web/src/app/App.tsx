import { useEffect, useState } from "react";

import { LoginDialog } from "../features/authentication/LoginDialog";
import { ProfileDialog } from "../features/authentication/ProfileDialog";
import { ConversionWorkspace } from "../features/conversion-workspace/ConversionWorkspace";
import { PreviewPage } from "../features/conversion-workspace/PreviewPage";
import type { UserProfile } from "../shared/api/auth-api";
import type { ConversionItem } from "../shared/api/conversion-api";
import { createTask, getTasks, type TaskSummary } from "../shared/api/task-api";

type Page = "workspace" | "tasks" | "docs" | "preview";

function isUserProfile(value: unknown): value is UserProfile {
  return typeof value === "object" && value !== null
    && typeof (value as UserProfile).username === "string"
    && typeof (value as UserProfile).email === "string";
}

function UserAvatar({ user }: { user: UserProfile }) {
  return (
    <span className="avatar">
      {user.avatar_url ? <img src={user.avatar_url} alt="" /> : user.username.slice(0, 2).toUpperCase()}
    </span>
  );
}

function Docs() {
  return (
    <section className="docs-layout" aria-labelledby="docs-title">
      <aside className="docs-sidebar">
        <p>DOCUMENTATION</p>
        <a href="#overview">概览</a>
        <a href="#api">API 接口</a>
        <a href="#formats">支持格式</a>
        <a href="#local">本地开发</a>
      </aside>
      <article className="docs-content">
        <p className="eyebrow">DEVELOPER DOCUMENTS</p>
        <h1 id="docs-title">DocShift 接口与功能说明</h1>
        <p>后端以版本化 HTTP API 向前端提供用户、文件转换、预览与导出能力。用户可为每个文件选择目标格式，转换结果在确认导出前始终保留在当前 Task 中。</p>
        <h2 id="api">核心 API</h2>
        <div className="api-table">
          <code>GET</code><span>/api/v1/health</span><p>服务进程存活检查。</p>
          <code>GET</code><span>/api/v1/tasks</span><p>读取当前用户的 Task 列表与文件数量汇总。</p>
          <code>POST</code><span>/api/v1/tasks</span><p>创建一个新的文档转换 Task。</p>
          <code>GET</code><span>/api/v1/tasks/{"{task_id}"}</span><p>读取指定 Task 与其中的转换文件。</p>
          <code>POST</code><span>/api/v1/tasks/{"{task_id}"}/items</span><p>为指定 Task 创建一个转换项。</p>
          <code>GET</code><span>/api/v1/conversions/{"{conversion_id}"}/preview</span><p>预览源文件或转换结果。</p>
        </div>
        <h2 id="formats">支持格式</h2>
        <p>DOCX、PPTX、XLSX、HTML 与 PDF 可转换为 Markdown；Markdown 可转换为 DOCX、HTML 或 XLSX。PDF 转 DOCX 不在当前范围内。</p>
        <h2 id="local">本地开发</h2>
        <pre><code>scl enable devtoolset-9 -- /path/to/docshift_server{"\n"}cd web && pnpm dev</code></pre>
      </article>
    </section>
  );
}

function Sidebar({ page, hasTask, onNavigate }: { page: Page; hasTask: boolean; onNavigate: (page: Page) => void }) {
  return (
    <aside className="app-sidebar" aria-label="主导航">
      <button className="brand" type="button" onClick={() => onNavigate("workspace")}>
        <span className="brand-mark" aria-hidden="true">D</span>
        <span>DocShift</span>
      </button>

      <p className="nav-group-label">WORKSPACE</p>
      <nav>
        <button className={page === "workspace" || page === "preview" ? "nav-link active" : "nav-link"} type="button" disabled={!hasTask} onClick={() => onNavigate("workspace")}>
          <span className="nav-symbol grid" aria-hidden="true" />任务工作台
        </button>
        <button className={page === "tasks" ? "nav-link active" : "nav-link"} type="button" onClick={() => onNavigate("tasks")}><span className="nav-symbol list" aria-hidden="true" />全部任务</button>
        <span className="nav-link muted" aria-disabled="true"><span className="nav-symbol archive" aria-hidden="true" />最近导出 <small>即将支持</small></span>
      </nav>

      <p className="nav-group-label">RESOURCES</p>
      <nav>
        <button className={page === "docs" ? "nav-link active" : "nav-link"} type="button" onClick={() => onNavigate("docs")}>
          <span className="nav-symbol book" aria-hidden="true" />接口文档
        </button>
      </nav>
    </aside>
  );
}

function Topbar({ page, task, user, onOpenProfile }: { page: Page; task: TaskSummary | null; user: UserProfile; onOpenProfile: () => void }) {
  const pageInfo = page === "docs"
    ? ["资源中心", "接口文档"]
    : page === "preview"
      ? ["任务工作台", "转换结果预览"]
      : ["任务工作台", "当前任务"];

  return (
    <header className="workspace-topbar">
      <p><strong>{pageInfo[0]}</strong><span>/</span>{pageInfo[1]}</p>
      <div className="topbar-actions">
        {page === "workspace" && task !== null ? <span className="topbar-status"><i aria-hidden="true" />{task.display_name}</span> : null}
        <button className="topbar-account" type="button" onClick={onOpenProfile} aria-label="打开个人资料">
          <UserAvatar user={user} />
          <span>{user.username}</span>
        </button>
      </div>
    </header>
  );
}

function WorkspacePage({ task, onPreview }: { task: TaskSummary; onPreview: (item: ConversionItem) => void }) {
  return (
    <section className="workspace-page" aria-labelledby="workspace-page-title">
      <div className="workspace-main">
        <section className="workspace-intro">
          <div>
            <p className="eyebrow">DOCSHIFT WORKSPACE</p>
            <h1 id="workspace-page-title">把一次转换，放进一个清晰的工作流</h1>
            <p>在同一个 Task 内完成文件上传、转换、预览与导出；每份文件独立处理，结果由你确认后再下载。</p>
          </div>
          <dl className="workspace-highlights">
            <div><dt>多文件</dt><dd>独立转换</dd></div>
            <div><dt>预览确认</dt><dd>再导出本地</dd></div>
            <div><dt>结果集中</dt><dd>按 Task 管理</dd></div>
          </dl>
        </section>

        <div className="workbench-layout">
          <ConversionWorkspace taskId={task.task_id} onPreview={onPreview} />
          <aside className="context-column" aria-label="当前任务摘要">
            <article className="task-context panel">
              <p className="panel-label">CURRENT TASK</p>
              <h2>{task.display_name}</h2>
              <p>新加入的文件会保留在当前任务中，等待单独设置目标格式。</p>
              <div className="task-context-line"><span>文件入队</span><span>选择格式</span><span>预览结果</span></div>
            </article>
            <article className="quick-help panel">
              <p className="panel-label">HOW IT WORKS</p>
              <h2>简单三步</h2>
              <ol>
                <li>拖放或选择一个或多个文件</li>
                <li>为每个文件设置目标格式</li>
                <li>转换后预览，再选择导出</li>
              </ol>
            </article>
          </aside>
        </div>
      </div>
    </section>
  );
}

function TaskListPage({ currentTaskId, error, loading, tasks, onOpen, onCreated }: {
  currentTaskId: string | null;
  error: string;
  loading: boolean;
  tasks: TaskSummary[];
  onOpen: (task: TaskSummary) => void;
  onCreated: (task: TaskSummary) => void;
}) {
  const [displayName, setDisplayName] = useState("");
  const [message, setMessage] = useState("");
  const [creating, setCreating] = useState(false);

  async function submit(): Promise<void> {
    setCreating(true);
    setMessage("");
    try {
      onCreated(await createTask(displayName.trim()));
      setDisplayName("");
    } catch (error) {
      setMessage(error instanceof Error ? error.message : "创建 Task 失败");
    } finally {
      setCreating(false);
    }
  }

  return <section className="task-list-page content-page" aria-labelledby="tasks-title">
    <p className="eyebrow">TASKS</p>
    <h1 id="tasks-title">管理你的转换任务</h1>
    <p className="task-list-copy">每个 Task 独立保存待转换文件、预览结果与导出记录。</p>
    <section className="task-create panel">
      <div><h2>创建 Task</h2><p>名称可选；未填写时将使用“未命名任务”。</p></div>
      <div className="task-create-form"><input value={displayName} maxLength={64} placeholder="例如：2026 年第一季度周报" onChange={(event) => setDisplayName(event.target.value)} /><button className="primary-button" type="button" disabled={creating} onClick={() => void submit()}>{creating ? "创建中…" : "新建 Task"}</button></div>
      {message ? <p className="workspace-message">{message}</p> : null}
    </section>
    <section className="task-list-section">
      <div className="section-title"><h2>全部任务</h2><span>{tasks.length}</span></div>
      {loading ? <p className="task-empty">正在加载 Task…</p> : null}
      {!loading && error ? <p className="workspace-message">{error}</p> : null}
      {!loading && tasks.length === 0 ? <p className="task-empty">还没有 Task，先创建一个开始转换。</p> : null}
      <div className="task-card-list">
        {tasks.map((task) => <button className={task.task_id === currentTaskId ? "task-card active" : "task-card"} type="button" key={task.task_id} onClick={() => onOpen(task)}>
          <div><strong>{task.display_name}</strong><span>{task.counts.total} 个文件 · {task.counts.preview_ready} 个已完成</span></div>
          <small>{task.counts.processing > 0 ? `${task.counts.processing} 个处理中` : task.counts.failed > 0 ? `${task.counts.failed} 个失败` : "查看工作台"}</small>
        </button>)}
      </div>
    </section>
  </section>;
}

export function App() {
  const [page, setPage] = useState<Page>("workspace");
  const [profileOpen, setProfileOpen] = useState(false);
  const [user, setUser] = useState<UserProfile | null>(() => {
    try {
      const rawUser = window.localStorage.getItem("docshift-demo-user");
      const savedUser = rawUser === null ? null : JSON.parse(rawUser);
      return isUserProfile(savedUser) ? savedUser : null;
    } catch {
      return null;
    }
  });
  const [previewItem, setPreviewItem] = useState<ConversionItem | null>(null);
  const [tasks, setTasks] = useState<TaskSummary[]>([]);
  const [tasksError, setTasksError] = useState("");
  const [tasksLoading, setTasksLoading] = useState(false);
  const [selectedTaskId, setSelectedTaskId] = useState<string | null>(null);

  useEffect(() => {
    if (user === null) {
      return;
    }

    let active = true;
    setTasksLoading(true);
    setTasksError("");
    void getTasks().then((result) => {
      if (!active) {
        return;
      }
      setTasks(result.items);
      setSelectedTaskId((current) => current ?? (result.items[0]?.task_id ?? null));
    }).catch(() => {
      if (active) {
        setTasksError("无法加载 Task，请确认后端服务正在运行。");
      }
    }).finally(() => {
      if (active) {
        setTasksLoading(false);
      }
    });
    return () => { active = false; };
  }, [user]);

  function updateUser(nextUser: UserProfile): void {
    try {
      window.localStorage.setItem("docshift-demo-user", JSON.stringify(nextUser));
    } catch {
      // The current session remains usable when browser storage is unavailable.
    }
    setUser(nextUser);
  }

  function openTask(task: TaskSummary): void {
    setSelectedTaskId(task.task_id);
    setPreviewItem(null);
    setPage("workspace");
  }

  function taskCreated(task: TaskSummary): void {
    setTasks((current) => [task, ...current]);
    openTask(task);
  }

  if (user === null) {
    return <LoginDialog onLoggedIn={updateUser} />;
  }

  const selectedTask = tasks.find((task) => task.task_id === selectedTaskId) ?? null;

  return (
    <div className="app-shell">
      <Sidebar page={page} hasTask={selectedTask !== null} onNavigate={setPage} />
      <main className="app-content">
        <Topbar page={page} task={selectedTask} user={user} onOpenProfile={() => setProfileOpen(true)} />
        {page === "tasks" || selectedTask === null ? <TaskListPage currentTaskId={selectedTaskId} error={tasksError} loading={tasksLoading} tasks={tasks} onOpen={openTask} onCreated={taskCreated} /> : null}
        {page === "workspace" && selectedTask !== null ? <WorkspacePage task={selectedTask} onPreview={(item) => { setPreviewItem(item); setPage("preview"); }} /> : null}
        {page === "docs" ? <div className="content-page"><Docs /></div> : null}
        {page === "preview" && previewItem !== null ? <div className="content-page preview-content"><PreviewPage item={previewItem} onBack={() => setPage("workspace")} /></div> : null}
      </main>
      {profileOpen ? <ProfileDialog user={user} onClose={() => setProfileOpen(false)} onSaved={(nextUser) => { updateUser(nextUser); setProfileOpen(false); }} /> : null}
    </div>
  );
}
