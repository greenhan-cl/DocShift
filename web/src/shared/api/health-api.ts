export interface HealthResponse {
  service: string;
  status: string;
  version: string;
}

const apiBaseUrl = (import.meta.env.VITE_API_BASE_URL || "/api/v1").replace(/\/+$/, "");

function isHealthResponse(value: unknown): value is HealthResponse {
  if (typeof value !== "object" || value === null) {
    return false;
  }

  const candidate = value as Record<string, unknown>;
  return (
    typeof candidate.service === "string" &&
    typeof candidate.status === "string" &&
    typeof candidate.version === "string"
  );
}

export async function getHealth(signal?: AbortSignal): Promise<HealthResponse> {
  const response = await fetch(`${apiBaseUrl}/health`, {
    method: "GET",
    credentials: "same-origin",
    headers: {
      Accept: "application/json",
    },
    signal,
  });

  if (!response.ok) {
    throw new Error(`Health request failed with status ${response.status}`);
  }

  const payload: unknown = await response.json();
  if (!isHealthResponse(payload)) {
    throw new Error("Health response has an invalid shape");
  }

  return payload;
}

