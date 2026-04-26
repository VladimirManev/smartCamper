/**
 * LED strip / relay detail modal: arc dimmer + light mode (presets / RGB hue / rainbow).
 */

import { useCallback, useEffect, useRef, useState } from "react";
import { LEDCard } from "./LEDCard";
import { LEDModalArcDimmer } from "./LEDModalArcDimmer";
import { CustomDropdown } from "./CustomDropdown";

const APPLIANCE_ICON_BY_TITLE = {
  Audio: "audio",
  Pump: "pump",
  Fridge: "fridge",
  "WC Fan": "fan",
  Boiler: "boiler",
  Inverter: "bulb",
};

const BRIGHTNESS_DEBOUNCE_MS = 150;
const BRIGHTNESS_MATCH_EPS = 3;

/** Channel recipes for white presets (RGBW); tweak to match your strips. */
export const LED_WHITE_PRESETS = {
  warm: { r: 255, g: 195, b: 140, w: 140 },
  neutral: { r: 255, g: 255, b: 252, w: 255 },
  cool: { r: 210, g: 230, b: 255, w: 110 },
};

const LIGHT_MODE = {
  WARM: "warm_white",
  NEUTRAL: "neutral_white",
  COOL: "cool_white",
  RGB: "rgb",
  RAINBOW: "rainbow",
};

const LIGHT_MODE_OPTIONS = [
  { value: LIGHT_MODE.WARM, label: "Warm white" },
  { value: LIGHT_MODE.NEUTRAL, label: "Neutral white" },
  { value: LIGHT_MODE.COOL, label: "Cool white" },
  { value: LIGHT_MODE.RGB, label: "RGB" },
  { value: LIGHT_MODE.RAINBOW, label: "Rainbow (static)" },
];

function clamp255(n) {
  return Math.min(255, Math.max(0, Math.round(Number(n)) || 0));
}

/** Same 0–359 hue → RGB as module-2 firmware (full saturation). */
export function hueToRgbChannels(hue) {
  let h = hue % 360;
  if (h < 0) h += 360;
  const region = Math.floor(h / 60);
  const rem = ((h % 60) * 255) / 60;
  const p = 0;
  const q = 255 - rem;
  const t = rem;
  let r;
  let g;
  let b;
  switch (region) {
    case 0:
      r = 255;
      g = t;
      b = p;
      break;
    case 1:
      r = q;
      g = 255;
      b = p;
      break;
    case 2:
      r = p;
      g = 255;
      b = t;
      break;
    case 3:
      r = p;
      g = q;
      b = 255;
      break;
    case 4:
      r = t;
      g = p;
      b = 255;
      break;
    default:
      r = 255;
      g = p;
      b = q;
      break;
  }
  return { r, g, b, w: 0 };
}

function rgbToHue(r, g, b) {
  const rn = clamp255(r) / 255;
  const gn = clamp255(g) / 255;
  const bn = clamp255(b) / 255;
  const max = Math.max(rn, gn, bn);
  const min = Math.min(rn, gn, bn);
  const d = max - min;
  if (d < 0.001) return 0;
  let h;
  if (max === rn) h = ((gn - bn) / d + (gn < bn ? 6 : 0)) / 6;
  else if (max === gn) h = (bn - rn) / d + 2;
  else h = (rn - gn) / d + 4;
  h *= 360;
  return (h + 360) % 360;
}

function channelsMatchPreset(c, preset, tol) {
  return (
    Math.abs(clamp255(c.r) - preset.r) <= tol &&
    Math.abs(clamp255(c.g) - preset.g) <= tol &&
    Math.abs(clamp255(c.b) - preset.b) <= tol &&
    Math.abs(clamp255(c.w ?? 0) - preset.w) <= tol
  );
}

function inferLightMode(strip) {
  if (!strip) return LIGHT_MODE.NEUTRAL;
  if (strip.effect === "rainbow_static") return LIGHT_MODE.RAINBOW;
  const c = strip.channels || {};
  if (channelsMatchPreset(c, LED_WHITE_PRESETS.warm, 30)) return LIGHT_MODE.WARM;
  if (channelsMatchPreset(c, LED_WHITE_PRESETS.neutral, 30)) return LIGHT_MODE.NEUTRAL;
  if (channelsMatchPreset(c, LED_WHITE_PRESETS.cool, 30)) return LIGHT_MODE.COOL;
  return LIGHT_MODE.RGB;
}

export function LEDStripModalContent({
  variant,
  strip,
  stripIndex,
  onBrightness,
  onToggle,
  disabled,
  icon = "bulb",
  sendStripApply,
  relayStyle = "default",
}) {
  const debounceRef = useRef(null);
  const brightnessForApplyRef = useRef(1);
  const hueBarRef = useRef(null);
  const hueDragRef = useRef(false);
  const [sliderOverride, setSliderOverride] = useState(null);
  const [lightMode, setLightMode] = useState(LIGHT_MODE.NEUTRAL);
  const [hueLocal, setHueLocal] = useState(0);

  useEffect(() => {
    return () => {
      if (debounceRef.current) clearTimeout(debounceRef.current);
    };
  }, []);

  useEffect(() => {
    setLightMode(inferLightMode(strip));
    const c = strip?.channels || { r: 255, g: 255, b: 255 };
    setHueLocal(rgbToHue(c.r, c.g, c.b));
  }, [stripIndex]);

  const brightness = strip?.brightness ?? 0;

  useEffect(() => {
    if (sliderOverride === null) return;
    if (Math.abs(brightness - sliderOverride) <= BRIGHTNESS_MATCH_EPS) {
      setSliderOverride(null);
    }
  }, [brightness, sliderOverride]);

  const scheduleBrightness = useCallback(
    (value) => {
      if (debounceRef.current) clearTimeout(debounceRef.current);
      debounceRef.current = setTimeout(() => {
        debounceRef.current = null;
        onBrightness(value);
      }, BRIGHTNESS_DEBOUNCE_MS);
    },
    [onBrightness]
  );

  const handleBrightnessCommit = useCallback(
    (value) => {
      if (debounceRef.current) {
        clearTimeout(debounceRef.current);
        debounceRef.current = null;
      }
      setSliderOverride(value);
      onBrightness(value);
    },
    [onBrightness]
  );

  const sliderValue = Math.min(
    255,
    Math.max(1, (sliderOverride !== null ? sliderOverride : brightness) || 1)
  );
  brightnessForApplyRef.current = sliderValue;

  const applyPayloadBase = useCallback(() => {
    const base = {
      brightness: Math.max(1, brightnessForApplyRef.current || 1),
    };
    if (stripIndex !== 3) base.mode = "on";
    return base;
  }, [stripIndex]);

  const sendRgbHue = useCallback(
    (hue) => {
      if (typeof sendStripApply !== "function" || disabled) return;
      const { r, g, b, w } = hueToRgbChannels(hue);
      sendStripApply({
        ...applyPayloadBase(),
        effect: "normal",
        channels: { r, g, b, w },
      });
    },
    [sendStripApply, disabled, applyPayloadBase]
  );

  const applyHueFromClientX = useCallback(
    (clientX) => {
      const el = hueBarRef.current;
      if (!el) return;
      const rect = el.getBoundingClientRect();
      if (rect.width <= 0) return;
      const x = Math.min(Math.max(0, clientX - rect.left), rect.width);
      const hue = (x / rect.width) * 360;
      setHueLocal(hue);
      sendRgbHue(hue);
    },
    [sendRgbHue]
  );

  const applyLightMode = useCallback(
    (v) => {
      setLightMode(v);
      if (typeof sendStripApply !== "function" || disabled) return;

      const base = applyPayloadBase();

      if (v === LIGHT_MODE.RAINBOW) {
        sendStripApply({ ...base, effect: "rainbow_static" });
        return;
      }
      if (v === LIGHT_MODE.WARM) {
        sendStripApply({
          ...base,
          effect: "normal",
          channels: { ...LED_WHITE_PRESETS.warm },
        });
        return;
      }
      if (v === LIGHT_MODE.NEUTRAL) {
        sendStripApply({
          ...base,
          effect: "normal",
          channels: { ...LED_WHITE_PRESETS.neutral },
        });
        return;
      }
      if (v === LIGHT_MODE.COOL) {
        sendStripApply({
          ...base,
          effect: "normal",
          channels: { ...LED_WHITE_PRESETS.cool },
        });
        return;
      }
      if (v === LIGHT_MODE.RGB) {
        const c = strip?.channels || {};
        const h = rgbToHue(c.r, c.g, c.b);
        sendRgbHue(h);
      }
    },
    [sendStripApply, disabled, applyPayloadBase, sendRgbHue, strip?.channels]
  );

  if (variant === "relay") {
    const relayIsOn = strip?.state === "ON";
    const bulbIcon = (
      <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
        <path
          d="M12 2C9.24 2 7 4.24 7 7c0 1.57.8 2.95 2 3.74V14c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-3.26c1.2-.79 2-2.17 2-3.74 0-2.76-2.24-5-5-5z"
          stroke="currentColor"
          strokeWidth="2"
          fill={relayIsOn ? "currentColor" : "none"}
          strokeLinecap="round"
          strokeLinejoin="round"
        />
        <line x1="9" y1="18" x2="15" y2="18" stroke="currentColor" strokeWidth="2" strokeLinecap="round" />
        <line x1="10" y1="21" x2="14" y2="21" stroke="currentColor" strokeWidth="2" strokeLinecap="round" />
      </svg>
    );

    if (relayStyle === "ambient") {
      return (
        <div
          className={`led-strip-modal led-strip-modal--ambient-relay ${
            disabled ? "led-strip-modal--ambient-relay-disabled" : ""
          } ${relayIsOn ? "is-on" : "is-off"}`}
        >
          <div className="led-modal-ambient__frame">
            <svg
              className="led-modal-ambient__ring"
              viewBox="0 0 200 200"
              aria-hidden
            >
              <circle
                className="led-modal-ambient__ring-track"
                cx="100"
                cy="100"
                r="80"
              />
              <circle
                className="led-modal-ambient__ring-active"
                cx="100"
                cy="100"
                r="80"
              />
            </svg>

            <button
              type="button"
              className={`led-modal-arc-dimmer__bulb neumorphic-button ${
                relayIsOn ? "on" : "off"
              }`}
              disabled={disabled}
              onClick={onToggle}
              aria-label="Toggle ambient light"
            >
              <span className="button-text">
                <div
                  className={`icon-container ${
                    relayIsOn ? "icon-active" : "icon-inactive"
                  }`}
                >
                  {bulbIcon}
                </div>
              </span>
            </button>
          </div>
        </div>
      );
    }

    return (
      <div className="led-strip-modal led-strip-modal--relay">
        <div className="led-strip-modal__card">
          <LEDCard
            name=""
            strip={strip}
            onClick={onToggle}
            type="relay"
            disabled={disabled}
            icon={icon}
          />
        </div>
      </div>
    );
  }

  const hueMarkerPct = (hueLocal / 360) * 100;

  return (
    <div className="led-strip-modal led-strip-modal--strip">
      <LEDModalArcDimmer
        strip={strip}
        stripIndex={stripIndex}
        disabled={disabled}
        displayBrightness={sliderValue}
        onBrightnessDrag={(v) => {
          setSliderOverride(v);
          scheduleBrightness(v);
        }}
        onBrightnessCommit={handleBrightnessCommit}
        onToggle={onToggle}
      />
      {typeof sendStripApply === "function" && (
        <div className="led-strip-modal__light">
          <CustomDropdown
            className="led-strip-light-dropdown"
            value={lightMode}
            onChange={applyLightMode}
            options={LIGHT_MODE_OPTIONS}
            disabled={disabled}
            placeholder="Light mode"
            menuPlacement="top"
          />

          {lightMode === LIGHT_MODE.RGB && (
            <div className="led-strip-modal__hue-wrap">
              <div
                ref={hueBarRef}
                className="led-strip-modal__hue-bar"
                role="slider"
                tabIndex={disabled ? -1 : 0}
                aria-valuemin={0}
                aria-valuemax={360}
                aria-valuenow={Math.round(hueLocal)}
                aria-label="Hue"
                onKeyDown={(e) => {
                  if (disabled) return;
                  let delta = 0;
                  if (e.key === "ArrowLeft" || e.key === "ArrowDown") delta = -4;
                  else if (e.key === "ArrowRight" || e.key === "ArrowUp") delta = 4;
                  else return;
                  e.preventDefault();
                  const next = (hueLocal + delta + 360) % 360;
                  setHueLocal(next);
                  sendRgbHue(next);
                }}
                onPointerDown={(e) => {
                  if (disabled) return;
                  hueDragRef.current = true;
                  e.currentTarget.setPointerCapture(e.pointerId);
                  applyHueFromClientX(e.clientX);
                }}
                onPointerMove={(e) => {
                  if (disabled || !hueDragRef.current) return;
                  applyHueFromClientX(e.clientX);
                }}
                onPointerUp={(e) => {
                  hueDragRef.current = false;
                  try {
                    e.currentTarget.releasePointerCapture(e.pointerId);
                  } catch {
                    /* ignore */
                  }
                }}
                onPointerCancel={() => {
                  hueDragRef.current = false;
                }}
              >
                <div className="led-strip-modal__hue-bar-track" />
                <div
                  className="led-strip-modal__hue-marker"
                  style={{ left: `${hueMarkerPct}%` }}
                />
              </div>
            </div>
          )}
        </div>
      )}
    </div>
  );
}

export function getApplianceModalIcon(cardName) {
  return APPLIANCE_ICON_BY_TITLE[cardName] || "bulb";
}
