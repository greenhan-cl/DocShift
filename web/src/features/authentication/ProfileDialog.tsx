import { useRef, useState } from "react";

import { updateUserProfile, type UserProfile } from "../../shared/api/auth-api";

interface ProfileDialogProps {
  onClose: () => void;
  onSaved: (user: UserProfile) => void;
  user: UserProfile;
}

function initials(username: string): string {
  return username.trim().slice(0, 2).toUpperCase() || "DS";
}

const allowedAvatarTypes = new Set(["image/jpeg", "image/png", "image/webp"]);

export function ProfileDialog({ onClose, onSaved, user }: ProfileDialogProps) {
  const avatarInputRef = useRef<HTMLInputElement>(null);
  const [avatarUrl, setAvatarUrl] = useState(user.avatar_url ?? "");
  const [message, setMessage] = useState("");
  const [saving, setSaving] = useState(false);
  const [username, setUsername] = useState(user.username);

  function selectAvatar(file: File | undefined): void {
    if (file === undefined) {
      return;
    }
    if (!allowedAvatarTypes.has(file.type)) {
      setMessage("头像仅支持 PNG、JPG 或 WebP 图片。");
      return;
    }
    if (file.size > 1024 * 1024) {
      setMessage("演示头像最大支持 1 MB。");
      return;
    }

    const reader = new FileReader();
    reader.addEventListener("load", () => {
      setAvatarUrl(typeof reader.result === "string" ? reader.result : "");
      setMessage("");
    });
    reader.readAsDataURL(file);
  }

  async function saveProfile(): Promise<void> {
    const nextUsername = username.trim();
    if (nextUsername.length === 0) {
      setMessage("昵称不能为空。");
      return;
    }

    setSaving(true);
    setMessage("");
    try {
      const result = await updateUserProfile({ avatar_url: avatarUrl || null, username: nextUsername });
      onSaved(result.user);
    } catch (error) {
      setMessage(error instanceof Error ? error.message : "保存失败，请稍后重试。");
    } finally {
      setSaving(false);
    }
  }

  return (
    <div className="dialog-backdrop" role="presentation" onMouseDown={onClose}>
      <section className="profile-dialog" role="dialog" aria-modal="true" aria-labelledby="profile-title" onMouseDown={(event) => event.stopPropagation()}>
        <button className="dialog-close" type="button" onClick={onClose} aria-label="关闭">×</button>
        <p className="eyebrow">ACCOUNT SETTINGS</p>
        <h2 id="profile-title">个人资料</h2>
        <p className="profile-dialog-copy">修改后的资料只用于本地演示。头像会以浏览器数据形式保留，不会上传到真实服务端。</p>

        <div className="profile-avatar-editor">
          <button className="profile-avatar-button" type="button" onClick={() => avatarInputRef.current?.click()} aria-label="选择头像">
            {avatarUrl ? <img src={avatarUrl} alt="当前头像" /> : <span>{initials(username)}</span>}
            <i aria-hidden="true">更换</i>
          </button>
          <div><strong>个人头像</strong><span>支持 PNG、JPG、WebP，最大 1 MB</span></div>
          <input ref={avatarInputRef} type="file" accept="image/png,image/jpeg,image/webp" hidden onChange={(event) => selectAvatar(event.target.files?.[0])} />
        </div>

        <label>昵称<input value={username} maxLength={32} onChange={(event) => setUsername(event.target.value)} autoFocus /></label>
        <label>邮箱地址<input value={user.email} readOnly aria-readonly="true" /></label>
        {message ? <p className="profile-message">{message}</p> : null}
        <div className="profile-actions">
          <button className="secondary-button" type="button" onClick={onClose}>取消</button>
          <button className="primary-button" type="button" disabled={saving} onClick={() => void saveProfile()}>{saving ? "保存中…" : "保存资料"}</button>
        </div>
      </section>
    </div>
  );
}
