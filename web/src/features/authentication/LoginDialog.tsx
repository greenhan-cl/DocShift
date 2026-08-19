import { useState } from "react";

import { loginWithEmailCode, requestEmailCode, type UserProfile } from "../../shared/api/auth-api";

interface LoginDialogProps { onLoggedIn: (user: UserProfile) => void; }

export function LoginDialog({ onLoggedIn }: LoginDialogProps) {
  const [email, setEmail] = useState("");
  const [code, setCode] = useState("");
  const [sent, setSent] = useState(false);
  const [message, setMessage] = useState("");
  const [busy, setBusy] = useState(false);

  async function sendCode() {
    setBusy(true);
    setMessage("");
    try {
      const result = await requestEmailCode(email);
      setSent(true);
      setMessage(`验证码已发送。演示验证码：${result.demo_code}`);
    } catch (error) {
      setMessage(error instanceof Error ? error.message : "发送失败");
    } finally {
      setBusy(false);
    }
  }

  async function login() {
    setBusy(true);
    setMessage("");
    try {
      const result = await loginWithEmailCode(email, code);
      onLoggedIn(result.user);
    } catch (error) {
      setMessage(error instanceof Error ? error.message : "登录失败");
    } finally {
      setBusy(false);
    }
  }

  return (
    <main className="login-page">
      <section className="login-dialog login-page-card" aria-labelledby="login-title">
        <div className="login-product">
          <span className="login-logo" aria-hidden="true">D</span>
          <span>DocShift</span>
        </div>
        <p className="eyebrow">WELCOME TO DOCSHIFT</p>
        <h1 id="login-title">登录后开始转换</h1>
        <p>使用邮箱验证码登录，即可创建任务、转换文档并导出已确认的结果。</p>
        <label>
          邮箱地址
          <input type="email" value={email} placeholder="name@example.com" onChange={(event) => setEmail(event.target.value)} autoFocus />
        </label>
        {sent ? <label>验证码<input inputMode="numeric" maxLength={6} value={code} placeholder="输入 6 位验证码" onChange={(event) => setCode(event.target.value)} /></label> : null}
        {message ? <div className={message.includes("验证码已发送") ? "login-message success" : "login-message"}>{message}</div> : null}
        {sent ? (
          <button className="primary-button login-submit" type="button" disabled={busy || code.length !== 6} onClick={() => void login()}>{busy ? "验证中…" : "登录"}</button>
        ) : (
          <button className="primary-button login-submit" type="button" disabled={busy || email.length === 0} onClick={() => void sendCode()}>{busy ? "发送中…" : "发送验证码"}</button>
        )}
        <small>当前演示服务不会真实发送邮件。</small>
      </section>
    </main>
  );
}
