import { useCallback, useEffect, useRef, useState } from "react";

import {
  createConversion,
  createTaskExport,
  deleteConversion,
  downloadTaskExport,
  getConversions,
  type ConversionItem,
} from "../../shared/api/conversion-api";

const statusLabels: Record<ConversionItem["status"], string> = {
  queued: "排队中",
  processing: "转换中",
  preview_ready: "已转换",
  failed: "转换失败",
  expired: "结果已过期",
};

const formatTargets: Record<string, string[]> = {
  docx: ["markdown"],
  pptx: ["markdown"],
  xlsx: ["markdown"],
  pdf: ["markdown"],
  html: ["markdown"],
  htm: ["markdown"],
  md: ["docx", "html", "xlsx"],
};

function fileFormat(file: File): string {
  const part = file.name.split(".").pop();
  return part?.toLowerCase() ?? "file";
}

function size(bytes: number): string {
  return bytes > 1024 * 1024 ? `${(bytes / 1024 / 1024).toFixed(1)} MB` : `${Math.max(1, Math.round(bytes / 1024))} KB`;
}

export function ConversionWorkspace({ taskId, onPreview }: { taskId: string; onPreview: (item: ConversionItem) => void }) {
  const input = useRef<HTMLInputElement>(null);
  const [items, setItems] = useState<ConversionItem[]>([]);
  const [queuedFiles, setQueuedFiles] = useState<File[]>([]);
  const [targets, setTargets] = useState<Record<string, string>>({});
  const [message, setMessage] = useState("");
  const [busy, setBusy] = useState(false);
  const [exporting, setExporting] = useState(false);

  const load = useCallback(async () => {
    try {
      setItems((await getConversions(taskId)).items);
    } catch {
      setMessage("无法连接后端服务，请确认 C++ 服务正在运行。");
    }
  }, [taskId]);

  useEffect(() => {
    void load();
  }, [load]);

  function queue(files: FileList | null): void {
    if (files === null) {
      return;
    }
    const next = [...files];
    setQueuedFiles((current) => [...current, ...next]);
    setTargets((current) => ({
      ...current,
      ...Object.fromEntries(next.map((file) => [file.name, formatTargets[fileFormat(file)]?.[0] ?? "markdown"])),
    }));
  }

  async function startConversions(): Promise<void> {
    if (queuedFiles.length === 0) {
      return;
    }
    setBusy(true);
    try {
      await Promise.all(queuedFiles.map((file) => createConversion(taskId, file, targets[file.name] ?? "markdown")));
      setQueuedFiles([]);
      setMessage("文件已提交到转换队列，可在下方查看进度与预览。");
      await load();
    } catch {
      setMessage("提交失败，请确认后端服务正在运行。");
    } finally {
      setBusy(false);
    }
  }

  async function exportCompleted(): Promise<void> {
    setExporting(true);
    try {
      const taskExport = await createTaskExport(taskId);
      await downloadTaskExport(taskExport);
      setMessage(`已开始下载 ${taskExport.filename}，其中包含 ${taskExport.item_count} 个已完成文件。`);
    } catch {
      setMessage("导出失败，请稍后重试。");
    } finally {
      setExporting(false);
    }
  }

  async function remove(item: ConversionItem): Promise<void> {
    if (window.confirm(`删除“${item.source.filename}”吗？`)) {
      await deleteConversion(item.item_id);
      await load();
    }
  }

  const pending = items.filter((item) => item.status !== "preview_ready");
  const completed = items.filter((item) => item.status === "preview_ready");

  return (
    <section className="workspace-panel panel" aria-labelledby="workspace-title">
      <div className="panel-heading">
        <div>
          <p className="panel-label">CONVERSIONS</p>
          <h2 id="workspace-title">文件转换</h2>
        </div>
        <div className="workspace-actions">
          <button className="secondary-button" type="button" onClick={() => void load()}>刷新状态</button>
          {completed.length > 0 ? <button className="primary-button" type="button" disabled={exporting} onClick={() => void exportCompleted()}>{exporting ? "导出准备中…" : `导出已完成 (${completed.length})`}</button> : null}
        </div>
      </div>

      <div className="drop-zone" onDragOver={(event) => event.preventDefault()} onDrop={(event) => { event.preventDefault(); queue(event.dataTransfer.files); }}>
        <strong>选择文件，逐个设置目标格式</strong>
        <span>支持 DOCX / PPTX / XLSX / PDF / HTML / Markdown；文件会先安全暂存到当前 Task。</span>
        <button className="primary-button" type="button" onClick={() => input.current?.click()}>选择文件</button>
        <input ref={input} type="file" multiple hidden onChange={(event) => queue(event.target.files)} />
      </div>

      {queuedFiles.length > 0 ? <div className="upload-queue">
        <div className="section-title"><h3>待提交文件</h3><span>{queuedFiles.length} 个文件</span></div>
        {queuedFiles.map((file) => <div className="queue-file" key={`${file.name}-${file.size}`}>
          <span>{file.name}</span><small>{fileFormat(file).toUpperCase()} · {size(file.size)}</small>
          <label>转换为<select value={targets[file.name] ?? "markdown"} onChange={(event) => setTargets((current) => ({ ...current, [file.name]: event.target.value }))}>{(formatTargets[fileFormat(file)] ?? ["markdown"]).map((target) => <option key={target} value={target}>{target.toUpperCase()}</option>)}</select></label>
        </div>)}
        <button className="primary-button submit-conversions" type="button" disabled={busy} onClick={() => void startConversions()}>{busy ? "提交中…" : "开始转换"}</button>
      </div> : null}

      <div className="conversion-area">
        <div className="conversion-column"><div className="section-title"><h3>待转换 / 处理中</h3><span>{pending.length}</span></div><ConversionList items={pending} empty="没有待处理文件" onPreview={onPreview} onDelete={remove} /></div>
        <div className="conversion-column"><div className="section-title"><h3>已转换文件</h3><span>{completed.length}</span></div><ConversionList items={completed} empty="转换完成的文件会显示在这里" onPreview={onPreview} onDelete={remove} /></div>
      </div>
      {message ? <p className="workspace-message">{message}</p> : null}
    </section>
  );
}

function ConversionList({ items, empty, onPreview, onDelete }: { items: ConversionItem[]; empty: string; onPreview: (item: ConversionItem) => void; onDelete: (item: ConversionItem) => void }) {
  return <ul className="conversion-list">
    {items.length === 0 ? <li className="empty-state">{empty}</li> : items.map((item) => <li key={item.item_id} className="conversion-item">
      <div className="file-icon">{item.source.format.toUpperCase()}</div>
      <div className="file-info"><strong>{item.source.filename}</strong><span>{size(item.source.size_bytes)} · {item.source.format.toUpperCase()} → {item.target_format.toUpperCase()}</span>{item.error ? <small>{item.error.message}</small> : null}</div>
      <span className={`conversion-status ${item.status}`}>{statusLabels[item.status]}</span>
      <div className="item-actions"><button className="text-button preview-button" type="button" onClick={() => onPreview(item)}>预览</button><button className="text-button" type="button" onClick={() => onDelete(item)}>删除</button></div>
    </li>)}
  </ul>;
}
