export type ConversionStatus = "queued" | "processing" | "preview_ready" | "failed" | "expired";

export interface ConversionItem {
  item_id: string;
  task_id: string;
  source: { filename: string; format: string; size_bytes: number };
  target_format: string;
  status: ConversionStatus;
  download_available: boolean;
  error: { code: string; message: string } | null;
}

export interface Preview {
  title: string;
  kind: "source" | "result";
  content: string;
}

interface TaskExport {
  export_id: string;
  filename: string;
  item_count: number;
  download_url: string;
}

const apiBaseUrl = (import.meta.env.VITE_API_BASE_URL || "/api/v1").replace(/\/+$/, "");

async function request<T>(path: string, init?: RequestInit): Promise<T> {
  const response = await fetch(`${apiBaseUrl}${path}`, {
    ...init,
    headers: { Accept: "application/json", ...init?.headers },
  });

  if (!response.ok) {
    throw new Error(`Request failed with status ${response.status}`);
  }
  if (response.status === 204) {
    return undefined as T;
  }

  const payload = await response.json() as { data?: T };
  if (payload.data === undefined) {
    throw new Error("API response has an invalid shape");
  }
  return payload.data;
}

export function getConversions(taskId: string): Promise<{ items: ConversionItem[] }> {
  return request(`/tasks/${taskId}`);
}

export function createConversion(taskId: string, file: File, targetFormat: string): Promise<ConversionItem> {
  const body = new FormData();
  body.append("file", file);
  body.append("target_format", targetFormat);

  return request(`/tasks/${taskId}/items`, {
    method: "POST",
    body,
  });
}

export function getPreview(itemId: string): Promise<Preview> {
  return request(`/conversions/${itemId}/preview`);
}

export function deleteConversion(itemId: string): Promise<void> {
  return request(`/conversions/${itemId}`, { method: "DELETE" });
}

export function createTaskExport(taskId: string): Promise<TaskExport> {
  return request(`/tasks/${taskId}/exports`, { method: "POST" });
}

export async function downloadTaskExport(taskExport: TaskExport): Promise<void> {
  const response = await fetch(`${apiBaseUrl}${taskExport.download_url}`);
  if (!response.ok) {
    throw new Error(`Export download failed with status ${response.status}`);
  }

  const objectUrl = URL.createObjectURL(await response.blob());
  const link = document.createElement("a");
  link.href = objectUrl;
  link.download = taskExport.filename;
  document.body.append(link);
  link.click();
  link.remove();
  window.setTimeout(() => URL.revokeObjectURL(objectUrl), 0);
}
