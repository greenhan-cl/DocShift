import { useState } from "react";

import { SystemHealthCard } from "../features/system-health/SystemHealthCard";
import { LoginDialog } from "../features/authentication/LoginDialog";
import { ConversionWorkspace } from "../features/conversion-workspace/ConversionWorkspace";
import { PreviewPage } from "../features/conversion-workspace/PreviewPage";
import type { ConversionItem } from "../shared/api/conversion-api";

const navigation = [
  { id: "home", label: "主页" },
  { id: "console", label: "控制台" },
  { id: "docs", label: "文档" },
] as const;

type Page = (typeof navigation)[number]["id"] | "preview";

function Home({ openConsole, openDocs }: { openConsole: () => void; openDocs: () => void }) {
  return <>
    <section className="home-hero">
      <div className="hero-copy-block">
        <p className="eyebrow">DOCUMENT CONVERSION PLATFORM</p>
        <h1>一个工作区，<br />清晰管理你的文档转换。</h1>
        <p>上传、转换、预览与导出集中在同一个 Task 中。每份文件都有独立状态，结果在确认前始终由你掌控。</p>
        <div className="hero-actions"><button className="primary-button" type="button" onClick={openConsole}>进入控制台 <span>→</span></button><button className="secondary-button" type="button" onClick={openDocs}>查看接口文档</button></div>
        <div className="metric-row"><div><strong>8</strong><span>已支持转换路径</span></div><div><strong>多文件</strong><span>一个 Task，独立状态</span></div><div><strong>本地优先</strong><span>结果确认后再导出</span></div></div>
      </div>
      <div className="hero-window" aria-label="控制台功能预览">
        <div className="window-top"><span className="window-dots"><i /><i /><i /></span><span>docshift / workspace</span></div>
        <div className="window-body"><div className="window-heading"><div><small>ACTIVE TASK</small><strong>文档转换工作区</strong></div><span className="window-tag">3 个文件</span></div>
          <div className="window-file ready"><b>DOCX</b><div><strong>项目说明.docx</strong><span>DOCX → Markdown</span></div><em>待预览</em></div>
          <div className="window-file running"><b>PPTX</b><div><strong>季度汇报.pptx</strong><span>PPTX → Markdown</span></div><em>转换中</em></div>
          <div className="code-sample"><span>$</span> docshift export --task current<br /><i>Preparing selected results…</i></div>
        </div>
      </div>
    </section>
    <section className="feature-strip"><article><span>01</span><h2>集中管理</h2><p>以 Task 组织一次完整的转换工作，不让文件散落在多个页面。</p></article><article><span>02</span><h2>独立状态</h2><p>每个文件独立排队、处理、预览和导出，失败互不影响。</p></article><article><span>03</span><h2>结果可控</h2><p>先在服务端暂存并预览，确认需要的结果后再下载到本地。</p></article></section>
  </>;
}

function Docs() {
  return <section className="docs-layout"><aside className="docs-sidebar"><p>DOCUMENTATION</p><a href="#overview">概览</a><a href="#api">API 接口</a><a href="#formats">支持格式</a><a href="#local">本地开发</a></aside><article className="docs-content"><p className="eyebrow">DEVELOPER DOCUMENTS</p><h1 id="overview">DocShift 接口与功能说明</h1><p>后端以版本化 HTTP API 向前端提供用户、文件转换、预览与导出能力。用户直接对每个文件选择目标格式，无需创建 Task。</p><h2 id="api">核心 API</h2><div className="api-table"><code>GET</code><span>/api/v1/health</span><p>服务进程存活检查。</p><code>GET</code><span>/api/v1/conversions</span><p>读取当前用户的转换文件列表。</p><code>POST</code><span>/api/v1/conversions</span><p>上传文件并指定目标格式。</p><code>GET</code><span>/api/v1/conversions/{`{conversion_id}`}/preview</span><p>预览源文件或转换结果。</p></div><h2 id="formats">支持格式</h2><p>DOCX、PPTX、XLSX、HTML 与 PDF 可转换为 Markdown；Markdown 可转换为 DOCX、HTML 或 XLSX。PDF 转 DOCX 不在当前范围内。</p><h2 id="local">本地开发</h2><pre><code>python mock_server/app.py{`\n`}cd web && pnpm dev</code></pre></article></section>;
}

export function App() {
  const [page, setPage] = useState<Page>("home");
  const [loginOpen, setLoginOpen] = useState(false);
  const [user, setUser] = useState<{ username: string; email: string } | null>(null);
  const [previewItem, setPreviewItem] = useState<ConversionItem | null>(null);
  const initials = user === null ? "登录" : user.username.slice(0, 2).toUpperCase();
  return <div className="app-shell"><header className="topbar"><button className="brand" type="button" onClick={() => setPage("home")}><span className="brand-mark">D</span><span>DocShift</span></button><nav aria-label="主导航">{navigation.map((item) => <button className={page === item.id ? "nav-link active" : "nav-link"} type="button" key={item.id} onClick={() => setPage(item.id)}>{item.label}</button>)}</nav><div className="user-menu"><span className="notification-dot" aria-label="有一条通知" /><button className={user === null ? "avatar login-avatar" : "avatar"} type="button" title={user === null ? "登录" : `${user.username} 的账户`} onClick={() => setLoginOpen(true)}>{initials}</button></div></header><main className="main-content">{page === "home" ? <Home openConsole={() => setPage("console")} openDocs={() => setPage("docs")} /> : null}{page === "console" ? <section className="console-page"><div className="page-heading"><div><p className="eyebrow">CONSOLE</p><h1>控制台</h1><p>逐个选择文件的目标格式，实时查看处理状态与转换结果。</p></div><span className="live-chip"><i /> Mock API 已连接</span></div><ConversionWorkspace onPreview={(item) => { setPreviewItem(item); setPage("preview"); }} /><section className="console-bottom"><SystemHealthCard /><article className="panel quick-help"><p className="panel-label">QUICK START</p><h2>开始文件转换</h2><ol><li>选择或拖入一个或多个文件</li><li>为每个文件选择目标格式</li><li>随时预览待转换文件与已转换结果</li></ol></article></section></section> : null}{page === "docs" ? <Docs /> : null}{page === "preview" && previewItem !== null ? <PreviewPage item={previewItem} onBack={() => setPage("console")} /> : null}</main>{loginOpen ? <LoginDialog onClose={() => setLoginOpen(false)} onLoggedIn={(nextUser) => { setUser(nextUser); setLoginOpen(false); }} /> : null}</div>;
}
