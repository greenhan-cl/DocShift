import { useCallback, useEffect, useState } from "react";

import { getHealth, type HealthResponse } from "../../shared/api/health-api";

type HealthState =
  | { status: "checking" }
  | { status: "available"; health: HealthResponse }
  | { status: "unavailable" };

function isAbortError(error: unknown): boolean {
  return error instanceof DOMException && error.name === "AbortError";
}

export function SystemHealthCard() {
  const [healthState, setHealthState] = useState<HealthState>({ status: "checking" });

  const checkHealth = useCallback(async (signal?: AbortSignal) => {
    setHealthState({ status: "checking" });

    try {
      const health = await getHealth(signal);
      setHealthState({ status: "available", health });
    } catch (error: unknown) {
      if (!isAbortError(error)) {
        setHealthState({ status: "unavailable" });
      }
    }
  }, []);

  useEffect(() => {
    const controller = new AbortController();
    void checkHealth(controller.signal);

    return () => {
      controller.abort();
    };
  }, [checkHealth]);

  const isAvailable = healthState.status === "available";
  const isChecking = healthState.status === "checking";

  return (
    <article className="panel health-panel" aria-live="polite">
      <div className="panel-heading">
        <div>
          <p className="panel-label">BACKEND</p>
          <h2>服务端连接</h2>
        </div>
        <span className={`status-indicator status-${healthState.status}`}>
          <span aria-hidden="true" />
          {isChecking ? "检测中" : isAvailable ? "连接正常" : "尚未连接"}
        </span>
      </div>

      <div className="health-body">
        {isAvailable ? (
          <dl className="health-details">
            <div>
              <dt>服务</dt>
              <dd>{healthState.health.service}</dd>
            </div>
            <div>
              <dt>版本</dt>
              <dd>{healthState.health.version}</dd>
            </div>
          </dl>
        ) : (
          <p>
            {isChecking
              ? "正在调用健康检查接口。"
              : "启动后端服务后，可在这里验证前后端基础链路。"}
          </p>
        )}
      </div>

      {!isAvailable && !isChecking ? (
        <button className="secondary-button" type="button" onClick={() => void checkHealth()}>
          重新检测
        </button>
      ) : null}
    </article>
  );
}

