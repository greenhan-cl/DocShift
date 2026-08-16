const apiBaseUrl = (import.meta.env.VITE_API_BASE_URL || "/api/v1").replace(/\/+$/, "");

async function post<T>(path: string, body: Record<string, string>): Promise<T> {
  const response = await fetch(`${apiBaseUrl}${path}`, { method: "POST", headers: { "Content-Type": "application/json", Accept: "application/json" }, body: JSON.stringify(body) });
  const payload: unknown = await response.json();
  if (!response.ok) {
    const message = typeof payload === "object" && payload !== null && "error" in payload ? (payload as { error?: { message?: string } }).error?.message : "请求失败，请稍后再试。";
    throw new Error(message);
  }
  return (payload as { data: T }).data;
}

export function requestEmailCode(email: string): Promise<{ demo_code: string }> { return post("/auth/email-verifications", { email, purpose: "login" }); }
export function loginWithEmailCode(email: string, verificationCode: string): Promise<{ user: { username: string; email: string } }> { return post("/auth/login", { email, verification_code: verificationCode }); }
