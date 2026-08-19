export interface TaskCounts {
  total: number;
  processing: number;
  preview_ready: number;
  failed: number;
}

export interface TaskSummary {
  task_id: string;
  display_name: string;
  counts: TaskCounts;
}

export interface TaskDetail<TItem> {
  task: TaskSummary;
  items: TItem[];
}

const apiBaseUrl = (import.meta.env.VITE_API_BASE_URL || "/api/v1").replace(/\/+$/, "");

async function request<T>(path: string, init?: RequestInit): Promise<T> {
  const response = await fetch(`${apiBaseUrl}${path}`, {
    ...init,
    headers: { Accept: "application/json", ...init?.headers },
  });
  const payload: unknown = await response.json();
  if (!response.ok) {
    const message = typeof payload === "object" && payload !== null
      ? (payload as { error?: { message?: string } }).error?.message
      : undefined;
    throw new Error(message ?? `请求失败，状态码 ${response.status}`);
  }
  return (payload as { data: T }).data;
}

export function getTasks(): Promise<{ items: TaskSummary[] }> {
  return request("/tasks");
}

export function createTask(displayName: string): Promise<TaskSummary> {
  return request("/tasks", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ display_name: displayName }),
  });
}
