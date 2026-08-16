import { useEffect, useState } from "react";
import { getPreview, type ConversionItem, type Preview } from "../../shared/api/conversion-api";

export function PreviewPage({ item, onBack }: { item: ConversionItem; onBack: () => void }) {
  const [preview, setPreview] = useState<Preview | null>(null);
  const [error, setError] = useState("");
  useEffect(() => { let active = true; void getPreview(item.item_id).then((result) => { if (active) setPreview(result); }).catch(() => { if (active) setError("无法加载预览内容，请稍后重试。"); }); return () => { active = false; }; }, [item.item_id]);
  return <section className="full-preview-page"><button className="back-button" type="button" onClick={onBack}>← 返回控制台</button><div className="preview-page-heading"><div><p className="eyebrow">{item.status === "preview_ready" ? "CONVERSION RESULT" : "SOURCE FILE"}</p><h1>{item.source.filename}</h1><p>{item.source.format.toUpperCase()} → {item.target_format.toUpperCase()} · {item.status === "preview_ready" ? "转换结果预览" : "源文件与转换进度预览"}</p></div><span className={`conversion-status ${item.status}`}>{item.status === "preview_ready" ? "已转换" : "处理中"}</span></div><article className="full-preview-card">{error ? <p className="workspace-message">{error}</p> : preview === null ? <p className="workspace-message">正在加载预览…</p> : <><div className="preview-card-bar"><span>{preview.kind === "result" ? "结果内容" : "源文件信息"}</span><span>只读预览</span></div><pre>{preview.content}</pre></>}</article></section>;
}
