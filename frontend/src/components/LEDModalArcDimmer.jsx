/**
 * Large horseshoe arc as brightness touch slider; center is toggle only (bulb or AUTO).
 */

import { useEffect, useRef, useState, useCallback } from "react";
import { getArcProgress } from "../utils/arcProgress";
import { getThemeColor } from "../utils/getThemeColor";

const ARC_PATH = "M 43.4 156.6 A 80 80 0 1 1 156.6 156.6";
const ARC_LENGTH = Math.PI * 80 * (270 / 180);
const HIT_DISTANCE = 32;

function clientToSvg(svg, clientX, clientY) {
  const pt = svg.createSVGPoint();
  pt.x = clientX;
  pt.y = clientY;
  const ctm = svg.getScreenCTM();
  if (!ctm) return { x: 0, y: 0 };
  return pt.matrixTransform(ctm.inverse());
}

function brightnessAtPathPoint(pathEl, x, y) {
  if (!pathEl) return null;
  const len = pathEl.getTotalLength();
  if (len <= 0) return null;
  let bestT = 0;
  let bestD = Infinity;
  const steps = 100;
  for (let i = 0; i <= steps; i++) {
    const t = (i / steps) * len;
    const p = pathEl.getPointAtLength(t);
    const d = (p.x - x) ** 2 + (p.y - y) ** 2;
    if (d < bestD) {
      bestD = d;
      bestT = t;
    }
  }
  if (Math.sqrt(bestD) > HIT_DISTANCE) return null;
  const ratio = bestT / len;
  return Math.min(255, Math.max(1, Math.round(1 + ratio * 254)));
}

export function LEDModalArcDimmer({
  strip,
  stripIndex,
  disabled,
  onBrightnessDrag,
  onBrightnessCommit,
  onToggle,
  displayBrightness,
}) {
  const svgRef = useRef(null);
  const hitPathRef = useRef(null);
  const draggingRef = useRef(false);
  const lastValueRef = useRef(displayBrightness);
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");

  const isOn = strip?.state === "ON";
  const mode = strip?.mode;
  const displayText = mode === "AUTO" ? "AUTO" : strip?.state || "OFF";
  const showIcon = displayText === "OFF" || displayText === "ON";
  const isIconOn = isOn && displayText === "ON";

  let buttonClass = "led-modal-arc-dimmer__bulb neumorphic-button";
  if (mode === "AUTO") {
    buttonClass += " auto";
  } else {
    buttonClass += isOn ? " on" : " off";
  }
  const iconClass = `icon-${isIconOn ? "active" : "inactive"}`;

  useEffect(() => {
    const tick = () => {
      setAccentBlue(getThemeColor("--color-accent-blue"));
      setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
    };
    tick();
    const id = setInterval(tick, 2000);
    return () => clearInterval(id);
  }, []);

  useEffect(() => {
    lastValueRef.current = displayBrightness;
  }, [displayBrightness]);

  const gradientId = `modal-arc-grad-${stripIndex}`;

  const progress = getArcProgress(displayBrightness, true);
  const arcOpacity = isOn ? 1 : 0.38;

  const endDrag = useCallback(
    (e) => {
      if (!draggingRef.current) return;
      draggingRef.current = false;
      const path = hitPathRef.current;
      if (path && e?.pointerId != null) {
        try {
          path.releasePointerCapture(e.pointerId);
        } catch {
          /* already released */
        }
      }
      onBrightnessCommit(lastValueRef.current);
    },
    [onBrightnessCommit]
  );

  const onPointerDown = (e) => {
    if (disabled) return;
    const svg = svgRef.current;
    const path = hitPathRef.current;
    if (!svg || !path) return;
    const { x, y } = clientToSvg(svg, e.clientX, e.clientY);
    const b = brightnessAtPathPoint(path, x, y);
    if (b === null) return;
    e.preventDefault();
    draggingRef.current = true;
    lastValueRef.current = b;
    path.setPointerCapture(e.pointerId);
    onBrightnessDrag(b);
  };

  const onPointerMove = (e) => {
    if (!draggingRef.current || disabled) return;
    const svg = svgRef.current;
    const path = hitPathRef.current;
    if (!svg || !path) return;
    const { x, y } = clientToSvg(svg, e.clientX, e.clientY);
    const b = brightnessAtPathPoint(path, x, y);
    if (b === null) return;
    lastValueRef.current = b;
    onBrightnessDrag(b);
  };

  const bulbIcon = (
    <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path
        d="M12 2C9.24 2 7 4.24 7 7c0 1.57.8 2.95 2 3.74V14c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-3.26c1.2-.79 2-2.17 2-3.74 0-2.76-2.24-5-5-5z"
        stroke="currentColor"
        strokeWidth="2"
        fill={isIconOn ? "currentColor" : "none"}
        strokeLinecap="round"
        strokeLinejoin="round"
      />
      <line x1="9" y1="18" x2="15" y2="18" stroke="currentColor" strokeWidth="2" strokeLinecap="round" />
      <line x1="10" y1="21" x2="14" y2="21" stroke="currentColor" strokeWidth="2" strokeLinecap="round" />
    </svg>
  );

  return (
    <div
      className={`led-modal-arc-dimmer ${disabled ? "led-modal-arc-dimmer--disabled" : ""}`}
      style={{ touchAction: "none" }}
    >
      <div className="led-modal-arc-dimmer__frame">
        <svg
          ref={svgRef}
          className="led-modal-arc-dimmer__svg"
          viewBox="0 0 200 200"
          role="slider"
          aria-valuemin={1}
          aria-valuemax={255}
          aria-valuenow={displayBrightness}
          aria-label="Brightness"
        >
          <defs>
            <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
              <stop offset="0%" stopColor={accentBlue} />
              <stop offset="100%" stopColor={accentBlueDark} />
            </linearGradient>
          </defs>
          <path
            d={ARC_PATH}
            fill="none"
            stroke="var(--color-text-secondary)"
            strokeWidth="10"
            strokeLinecap="round"
            opacity={0.22}
          />
          <path
            ref={hitPathRef}
            d={ARC_PATH}
            fill="none"
            stroke="transparent"
            strokeWidth="44"
            strokeLinecap="round"
            style={{ cursor: disabled ? "default" : "pointer", touchAction: "none" }}
            onPointerDown={onPointerDown}
            onPointerMove={onPointerMove}
            onPointerUp={endDrag}
            onPointerCancel={endDrag}
          />
          <path
            d={ARC_PATH}
            fill="none"
            stroke={`url(#${gradientId})`}
            strokeWidth="10"
            strokeLinecap="round"
            strokeDasharray={`${progress} ${ARC_LENGTH}`}
            strokeDashoffset="0"
            style={{
              pointerEvents: "none",
              opacity: progress > 0 ? arcOpacity : 0,
            }}
          />
        </svg>

        <button
          type="button"
          className={buttonClass}
          disabled={disabled}
          onClick={(e) => {
            e.stopPropagation();
            if (disabled) return;
            onToggle();
          }}
          aria-label={mode === "AUTO" ? "Cycle bathroom mode" : "Toggle light"}
        >
          <span className="button-text">
            {showIcon ? (
              <div className={`icon-container ${iconClass}`}>{bulbIcon}</div>
            ) : (
              <span className="led-modal-arc-dimmer__auto-text">{displayText}</span>
            )}
          </span>
        </button>
      </div>
      <div className="led-modal-arc-dimmer__value">{displayBrightness}</div>
    </div>
  );
}
