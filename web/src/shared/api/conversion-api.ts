export type ConversionStatus = "queued" | "processing" | "preview_ready" | "failed" | "expired";
export interface ConversionItem { item_id: string; source: { filename: string; format: string; size_bytes: number }; target_format: string; status: ConversionStatus; download_available: boolean; error: { code: string; message: string } | null; }
export interface Preview { title: string; kind: "source" | "result"; content: string; }
const apiBaseUrl = (import.meta.env.VITE_API_BASE_URL || "/api/v1").replace(/\/+$/, "");
async function request<T>(path: string, init?: RequestInit): Promise<T> { const response = await fetch(`${apiBaseUrl}${path}`, { ...init, headers: { Accept: "application/json", ...init?.headers } }); if (!response.ok) throw new Error(`Request failed with status ${response.status}`); if (response.status === 204) return undefined as T; const payload = await response.json() as { data?: T }; if (payload.data === undefined) throw new Error("API response has an invalid shape"); return payload.data; }
export function getConversions(): Promise<{ items: ConversionItem[] }> { return request("/conversions"); }
export function createConversion(file: File, targetFormat: string): Promise<ConversionItem> { return request("/conversions", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ filename: file.name, size_bytes: file.size, target_format: targetFormat }) }); }
export function getPreview(itemId: string): Promise<Preview> { return request(`/conversions/${itemId}/preview`); }
export function deleteConversion(itemId: string): Promise<void> { return request(`/conversions/${itemId}`, { method: "DELETE" }); }
