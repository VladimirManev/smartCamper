/**
 * SOC history for last 24h — simple SVG line chart (nested battery modal).
 */

import { useEffect, useMemo, useState } from "react";
import { fetchHistoryReadings } from "../utils/fetchHistoryReadings";

const CHART_W = 320;
const CHART_H = 180;
const PAD = { top: 16, right: 12, bottom: 28, left: 36 };

function formatHourLabel(ts) {
  const d = new Date(ts);
  const hh = String(d.getHours()).padStart(2, "0");
  const mm = String(d.getMinutes()).padStart(2, "0");
  return `${hh}:${mm}`;
}

/**
 * @param {Object} props
 * @param {number|null} [props.liveSoc] - current live SOC from socket (optional header)
 */
export function BatterySocHistoryModalContent({ liveSoc = null }) {
  const [status, setStatus] = useState("loading"); // loading | ready | empty | error
  const [points, setPoints] = useState([]);
  const [errorMessage, setErrorMessage] = useState("");

  useEffect(() => {
    const controller = new AbortController();

    (async () => {
      setStatus("loading");
      setErrorMessage("");
      try {
        const data = await fetchHistoryReadings({
          metric: "soc",
          hours: 24,
          signal: controller.signal,
        });
        const next = Array.isArray(data?.points) ? data.points : [];
        setPoints(next);
        setStatus(next.length === 0 ? "empty" : "ready");
      } catch (err) {
        if (err?.name === "AbortError") return;
        setErrorMessage(err?.message || "Failed to load");
        setStatus("error");
      }
    })();

    return () => controller.abort();
  }, []);

  const chart = useMemo(() => {
    if (points.length === 0) return null;

    const innerW = CHART_W - PAD.left - PAD.right;
    const innerH = CHART_H - PAD.top - PAD.bottom;
    const minTs = points[0].ts;
    const maxTs = points[points.length - 1].ts;
    const span = Math.max(maxTs - minTs, 1);

    const yMin = 0;
    const yMax = 100;

    const toX = (ts) => PAD.left + ((ts - minTs) / span) * innerW;
    const toY = (value) => {
      const clamped = Math.min(yMax, Math.max(yMin, Number(value)));
      return PAD.top + ((yMax - clamped) / (yMax - yMin)) * innerH;
    };

    const path = points
      .map((p, i) => {
        const x = toX(p.ts);
        const y = toY(p.value);
        return `${i === 0 ? "M" : "L"}${x.toFixed(1)},${y.toFixed(1)}`;
      })
      .join(" ");

    const yTicks = [0, 25, 50, 75, 100];
    const xLabels = [
      { ts: minTs, label: formatHourLabel(minTs) },
      { ts: minTs + span / 2, label: formatHourLabel(minTs + span / 2) },
      { ts: maxTs, label: formatHourLabel(maxTs) },
    ];

    const last = points[points.length - 1];

    return { path, toX, toY, yTicks, xLabels, last, innerH };
  }, [points]);

  const liveText =
    liveSoc != null && !Number.isNaN(Number(liveSoc))
      ? `${Math.round(Number(liveSoc))}%`
      : null;

  return (
    <div className="soc-history-modal">
      <div className="soc-history-modal__header">
        <span className="soc-history-modal__title">SOC · last 24h</span>
        {liveText && (
          <span className="soc-history-modal__live">Now {liveText}</span>
        )}
      </div>

      {status === "loading" && (
        <p className="soc-history-modal__status">Loading…</p>
      )}
      {status === "empty" && (
        <p className="soc-history-modal__status">
          No SOC history yet. Data appears after the backend logger runs.
        </p>
      )}
      {status === "error" && (
        <p className="soc-history-modal__status soc-history-modal__status--error">
          {errorMessage}
        </p>
      )}

      {status === "ready" && chart && (
        <div className="soc-history-modal__chart-wrap">
          <svg
            className="soc-history-modal__chart"
            viewBox={`0 0 ${CHART_W} ${CHART_H}`}
            role="img"
            aria-label="SOC over the last 24 hours"
          >
            {chart.yTicks.map((tick) => {
              const y = chart.toY(tick);
              return (
                <g key={tick}>
                  <line
                    className="soc-history-modal__grid"
                    x1={PAD.left}
                    y1={y}
                    x2={CHART_W - PAD.right}
                    y2={y}
                  />
                  <text
                    className="soc-history-modal__axis-label"
                    x={PAD.left - 6}
                    y={y}
                    textAnchor="end"
                    dominantBaseline="middle"
                  >
                    {tick}
                  </text>
                </g>
              );
            })}
            <path className="soc-history-modal__line" d={chart.path} fill="none" />
            <circle
              className="soc-history-modal__dot"
              cx={chart.toX(chart.last.ts)}
              cy={chart.toY(chart.last.value)}
              r="3.5"
            />
            {chart.xLabels.map((item) => (
              <text
                key={item.ts}
                className="soc-history-modal__axis-label"
                x={chart.toX(item.ts)}
                y={CHART_H - 8}
                textAnchor="middle"
              >
                {item.label}
              </text>
            ))}
          </svg>
          <p className="soc-history-modal__footer">
            Last sample {formatHourLabel(chart.last.ts)} ·{" "}
            {Math.round(chart.last.value)}%
          </p>
        </div>
      )}
    </div>
  );
}
