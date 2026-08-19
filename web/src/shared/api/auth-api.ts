const apiBaseUrl = (import.meta.env.VITE_API_BASE_URL || "/api/v1").replace(/\/+$/, "");

export interface UserProfile {
  email: string;
  avatar_url: string | null;
  username: string;
}

type ApiError = { error?: { message?: string } };

async function request<T>(method: "PATCH" | "POST", path: string, body: Record<string, string | null>): Promise<T> {
  const response = await fetch(`${apiBaseUrl}${path}`, {
    method,
    headers: { "Content-Type": "application/json", Accept: "application/json" },
    body: JSON.stringify(body),
  });
  const payload: unknown = await response.json();

  if (!response.ok) {
    const message = typeof payload === "object" && payload !== null ? (payload as ApiError).error?.message : undefined;
    throw new Error(message ?? "请求失败，请稍后再试。");
  }

  return (payload as { data: T }).data;
}

export function requestEmailCode(email: string): Promise<{ demo_code: string }> {
  return request("POST", "/auth/email-verifications", { email, purpose: "login" });
}

export function loginWithEmailCode(email: string, verificationCode: string): Promise<{ user: UserProfile }> {
  return request("POST", "/auth/login", { email, verification_code: verificationCode });
}

export function updateUserProfile(profile: Pick<UserProfile, "avatar_url" | "username">): Promise<{ user: UserProfile }> {
  return request("PATCH", "/users/me", profile);
}
