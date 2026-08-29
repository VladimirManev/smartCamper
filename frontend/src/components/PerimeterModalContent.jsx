/**
 * Perimeter watch modal — bird's-eye camper image, arm toggle left of van, zone overlays.
 */

import { useCallback, useEffect, useId, useState } from "react";
import { getThemeColor } from "../utils/getThemeColor";

/**
 * Coverage zones as % of the van layer (image rotated -90°: nose up, rear down).
 */
const ZONES = [
  { id: "front", left: 32, top: -18, width: 36, height: 14 },
  { id: "rear", left: 32, top: 103.5, width: 36, height: 14 },
  { id: "left_front", left: 20, top: 10, width: 14, height: 34 },
  { id: "left_rear", left: 20, top: 56, width: 14, height: 34 },
  { id: "right_front", left: 66, top: 10, width: 14, height: 34 },
  { id: "right_rear", left: 66, top: 56, width: 14, height: 34 },
];

const MOTION_CLEAR_MS = 5 * 60 * 1000;
const CAMPER_IMAGE = "/camper_birth_view.png";

/**
 * Bucket age for perimeter labels (short, delicate).
 * @param {number} ageMs
 * @returns {string|null} null when older than 5 min
 */
export function formatPerimeterMotionAge(ageMs) {
  if (ageMs < 0 || ageMs >= MOTION_CLEAR_MS) return null;
  if (ageMs < 5000) return "now";
  if (ageMs < 10000) return "5s";
  if (ageMs < 30000) return "10s";
  if (ageMs < 60000) return "30s";
  if (ageMs < 120000) return "1m";
  if (ageMs < 180000) return "2m";
  if (ageMs < 240000) return "3m";
  if (ageMs < 300000) return "4m";
  return "5m";
}

function PerimeterNoteIcon({ muted = false }) {
  return (
    <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path
        d="M9 18V6l11-2v12"
        stroke="currentColor"
        strokeWidth="2"
        strokeLinecap="round"
        strokeLinejoin="round"
      />
      <circle cx="7" cy="18" r="3" stroke="currentColor" strokeWidth="2" fill="none" />
      <circle cx="18" cy="16" r="3" stroke="currentColor" strokeWidth="2" fill="none" />
      {muted && (
        <path
          d="M4 5l16 14"
          stroke="currentColor"
          strokeWidth="2.25"
          strokeLinecap="round"
        />
      )}
    </svg>
  );
}

/**
 * @param {Object} props
 * @param {boolean} [props.disabled]
 * @param {boolean} [props.armed]
 * @param {Record<string, number|null>} [props.perimeterLastMotion]
 * @param {Function} [props.onToggleArm]
 * @param {Function} [props.onSimulateMotion]
 */
export function PerimeterModalContent({
  disabled = false,
  armed = false,
  perimeterLastMotion = {},
  onToggleArm,
  onSimulateMotion,
}) {
  const [now, setNow] = useState(() => Date.now());
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");
  const reactId = useId().replace(/:/g, "");
  const gradientId = `perimeter-arm-grad-${reactId}`;

  useEffect(() => {
    const updateColors = () => {
      setAccentBlue(getThemeColor("--color-accent-blue"));
      setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
    };
    updateColors();
    const interval = setInterval(updateColors, 2000);
    return () => clearInterval(interval);
  }, []);

  useEffect(() => {
    if (!armed) return undefined;
    const id = setInterval(() => setNow(Date.now()), 1000);
    return () => clearInterval(id);
  }, [armed]);

  const handleToggle = useCallback(() => {
    if (disabled || !onToggleArm) return;
    onToggleArm(!armed);
  }, [armed, disabled, onToggleArm]);

  const handleZoneTap = useCallback(
    (zoneId) => {
      if (disabled || !armed || !onSimulateMotion) return;
      onSimulateMotion(zoneId);
    },
    [armed, disabled, onSimulateMotion]
  );

  return (
    <div
      className={`perimeter-modal${disabled ? " perimeter-modal--disabled" : ""}`}
    >
      <div className="perimeter-modal__main">
        <button
          type="button"
          className={`neumorphic-button perimeter-modal__arm ${
            armed ? "on" : "off"
          }`}
          onClick={handleToggle}
          disabled={disabled}
          aria-pressed={armed}
          aria-label={armed ? "Disarm perimeter" : "Arm perimeter"}
        >
          <svg className="horseshoe-progress" viewBox="0 0 200 200" aria-hidden>
            <defs>
              <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
                <stop offset="0%" stopColor={accentBlue} />
                <stop offset="100%" stopColor={accentBlueDark} />
              </linearGradient>
            </defs>
            {armed && (
              <circle
                className="horseshoe-fill"
                cx="100"
                cy="100"
                r="80"
                fill="none"
                stroke={`url(#${gradientId})`}
                strokeWidth="8"
                strokeLinecap="round"
              />
            )}
          </svg>
          <span className="button-text">
            <div
              className={`icon-container ${armed ? "icon-active" : "icon-inactive"}`}
            >
              <PerimeterNoteIcon muted={!armed} />
            </div>
          </span>
        </button>

        <div className="perimeter-modal__stage">
          <div className="perimeter-modal__van-layer">
            {armed &&
              ZONES.map((zone) => {
                const ts = perimeterLastMotion[zone.id];
                const ageMs = ts != null ? now - ts : null;
                const ageLabel =
                  ageMs != null ? formatPerimeterMotionAge(ageMs) : null;
                const alert = ageLabel != null;

                return (
                  <button
                    key={zone.id}
                    type="button"
                    className={`perimeter-modal__zone perimeter-modal__zone--${zone.id}${
                      alert ? " perimeter-modal__zone--alert" : ""
                    }`}
                    style={{
                      left: `${zone.left}%`,
                      top: `${zone.top}%`,
                      width: `${zone.width}%`,
                      height: `${zone.height}%`,
                    }}
                    onClick={() => handleZoneTap(zone.id)}
                    disabled={disabled}
                    aria-label={`${zone.id}${alert ? `, motion ${ageLabel}` : ""}`}
                  >
                    {alert && (
                      <span className="perimeter-modal__age">{ageLabel}</span>
                    )}
                  </button>
                );
              })}

            <img
              className="perimeter-modal__van-img"
              src={CAMPER_IMAGE}
              alt=""
              draggable={false}
            />
          </div>
        </div>
      </div>

      <p className="perimeter-modal__hint">
        {disabled
          ? "Module offline"
          : armed
            ? "Watching · tap a zone to simulate motion"
            : "Perimeter off"}
      </p>
    </div>
  );
}
