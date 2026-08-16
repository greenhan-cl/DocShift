import { useState } from "react";

import { loginWithEmailCode, requestEmailCode } from "../../shared/api/auth-api";

interface LoginDialogProps { onClose: () => void; onLoggedIn: (user: { username: string; email: string }) => void; }

export function LoginDialog({ onClose, onLoggedIn }: LoginDialogProps) {
  const [email, setEmail] = useState("");
  const [code, setCode] = useState("");
  const [sent, setSent] = useState(false);
  const [message, setMessage] = useState("");
  const [busy, setBusy] = useState(false);
  async function sendCode() { setBusy(true); setMessage(""); try { const result = await requestEmailCode(email); setSent(true); setMessage(`验证码已发送。Mock 演示验证码：${result.demo_code}`); } catch (error) { setMessage(error instanceof Error ? error.message : "发送失败"); } finally { setBusy(false); } }
  async function login() { setBusy(true); setMessage(""); try { const result = await loginWithEmailCode(email, code); onLoggedIn(result.user); } catch (error) { setMessage(error instanceof Error ? error.message : "登录失败"); } finally { setBusy(false); } }
  return <div className="dialog-backdrop" role="presentation" onMouseDown={onClose}><section className="login-dialog" role="dialog" aria-modal="true" aria-labelledby="login-title" onMouseDown={(event) => event.stopPropagation()}><button className="dialog-close" type="button" onClick={onClose} aria-label="关闭">×</button><span className="login-logo">D</span><p className="eyebrow">WELCOME TO DOCSHIFT</p><h2 id="login-title">使用邮箱登录</h2><p>输入邮箱后获取一次性验证码，无需设置密码。</p><label>邮箱地址<input type="email" value={email} placeholder="name@example.com" onChange={(event) => setEmail(event.target.value)} autoFocus /></label>{sent ? <label>验证码<input inputMode="numeric" maxLength={6} value={code} placeholder="输入 6 位验证码" onChange={(event) => setCode(event.target.value)} /></label> : null}{message ? <div className={message.includes("验证码已发送") ? "login-message success" : "login-message"}>{message}</div> : null}{sent ? <button className="primary-button login-submit" type="button" disabled={busy || code.length !== 6} onClick={() => void login()}>{busy ? "验证中…" : "登录"}</button> : <button className="primary-button login-submit" type="button" disabled={busy || email.length === 0} onClick={() => void sendCode()}>{busy ? "发送中…" : "发送验证码"}</button>}<small>本地 mock 模式不会真实发送邮件。</small></section></div>;
}
