/**
 * Boiler detail: tank colored by the two temperatures, element graphic while
 * the relay is closed, power and setpoint controls.
 */

import { useEffect, useId, useRef, useState } from "react";
import { ModalStatSlots } from "./ModalStatSlots";
import { getThemeColor } from "../utils/getThemeColor";

const SETPOINT_MIN = 5;
const SETPOINT_MAX = 70;
const STEP_BUTTON_DEBOUNCE_MS = 120;
const COLOR_COLD = [8, 36, 110];
const COLOR_HOT = [220, 32, 24];
const COLOR_COLD_C = 18;
const COLOR_HOT_C = 35;
const BOTTOM_BLUE_START_C = 3;
const BOTTOM_BLUE_FULL_C = 10;

const ERROR_TEXT = {
  empty: "No water",
  sensor: "Sensor error",
  overtemp: "Overheating",
};

const ELEMENT_PATH = (() => {
  const y = 9;
  const amplitude = 5.5;
  const xStart = 2;
  const xEnd = 78;
  const lead = 8;
  const periods = 4;
  const waveStart = xStart + lead;
  const waveEnd = xEnd - lead;
  const steps = periods * 16;
  const commands = [`M${xStart} ${y}`, `L${waveStart} ${y}`];
  for (let i = 1; i <= steps; i++) {
    const t = i / steps;
    const x = waveStart + (waveEnd - waveStart) * t;
    const waveY = y - Math.sin(t * periods * Math.PI * 2) * amplitude;
    commands.push(`L${x.toFixed(2)} ${waveY.toFixed(2)}`);
  }
  commands.push(`L${xEnd} ${y}`);
  return commands.join(" ");
})();

function temperatureRgb(celsius) {
  const span =
    (Math.min(COLOR_HOT_C, Math.max(COLOR_COLD_C, Number(celsius) || 0)) -
      COLOR_COLD_C) /
    (COLOR_HOT_C - COLOR_COLD_C);
  return COLOR_COLD.map((cold, index) =>
    Math.round(cold + (COLOR_HOT[index] - cold) * span)
  );
}

function rgbString(channels) {
  return `rgb(${channels[0]}, ${channels[1]}, ${channels[2]})`;
}

function bottomRgb(topCelsius, bottomCelsius) {
  const topChannels = temperatureRgb(topCelsius);
  const gap = Math.abs(Number(topCelsius) - Number(bottomCelsius));
  if (gap <= BOTTOM_BLUE_START_C) {
    return topChannels;
  }
  const mix = Math.min(
    1,
    (gap - BOTTOM_BLUE_START_C) / (BOTTOM_BLUE_FULL_C - BOTTOM_BLUE_START_C)
  );
  return topChannels.map((channel, index) =>
    Math.round(channel + (COLOR_COLD[index] - channel) * mix)
  );
}

function formatTemp(value) {
  return value === null || value === undefined ? "—" : `${value}°`;
}

/**
 * @param {Object} props
 * @param {Object} props.boiler
 * @param {boolean} props.enabled - thermostat armed (appliance relay 4)
 * @param {boolean} props.powerDisabled
 * @param {boolean} props.stepsDisabled - not in maintain mode
 * @param {boolean} props.acSupply - inverter or shore power
 * @param {boolean} props.disabled - module offline
 * @param {boolean} [props.readOnly] - status slide: show the mode, do not send commands
 * @param {Function} props.onPower
 * @param {Function} props.onOnce
 * @param {Function} props.onSetpoint
 */
export function BoilerModalContent({
  boiler,
  enabled = false,
  powerDisabled = false,
  stepsDisabled = false,
  acSupply = false,
  disabled = false,
  readOnly = false,
  onPower,
  onOnce,
  onSetpoint,
}) {
  const setpoint = boiler?.setpoint ?? 60;
  const full = disabled ? null : boiler?.full;
  const top = disabled ? null : boiler?.top;
  const bottom = disabled ? null : boiler?.bottom;
  const relayOn = !disabled && boiler?.relay === true;
  const errorText = disabled ? null : ERROR_TEXT[boiler?.error] || null;
  const presetDisabled = (value) => {
    if (disabled) return true;
    if (boiler?.mode === "once" && boiler?.once === value) return false;
    if (bottom !== null && bottom !== undefined && bottom >= value) return true;
    return !acSupply;
  };

  const adjust = (next) => {
    if (disabled || next < SETPOINT_MIN || next > SETPOINT_MAX || next === setpoint) {
      return;
    }
    onSetpoint?.(next);
  };

  let fillStyle = { height: "0%" };
  if (full === true) {
    const topChannels =
      top === null
        ? bottom === null
          ? [30, 64, 140]
          : temperatureRgb(bottom)
        : temperatureRgb(top);
    const bottomChannels =
      top === null || bottom === null ? topChannels : bottomRgb(top, bottom);
    const bottomColor = rgbString(bottomChannels);
    const topColor = rgbString(topChannels);
    fillStyle = {
      height: "100%",
      background: `linear-gradient(to top, ${bottomColor}, ${topColor})`,
    };
  } else if (full === false) {
    fillStyle = { height: "50%", background: "#1e3a8a" };
  }

  return (
    <div className="gray-water-modal boiler-modal">
      <div className="gray-water-modal-tank-wrap">
        <div className="gray-water-modal-tank" aria-hidden="true">
          <div
            className={`gray-water-modal-fill boiler-modal__fill${
              full === false ? " boiler-modal__fill--empty" : ""
            }`}
            style={fillStyle}
          />
          <div className="boiler-modal__temps">
            <span className="boiler-modal__temp">{formatTemp(top)}</span>
            <span className="boiler-modal__temp">{formatTemp(bottom)}</span>
          </div>
          {relayOn && (
            <svg
              className="boiler-modal__element"
              viewBox="0 0 80 18"
              aria-hidden="true"
            >
              <path
                d={ELEMENT_PATH}
                fill="none"
                stroke="currentColor"
                strokeWidth="3"
                strokeLinecap="round"
              />
            </svg>
          )}
        </div>
        {(!readOnly || boiler?.mode === "maintain") && (
          <div className="boiler-modal__setpoint">
            {!readOnly && (
              <StepButton
                label="Decrease temperature"
                disabled={disabled || stepsDisabled || setpoint <= SETPOINT_MIN}
                onClick={() => adjust(setpoint - 1)}
              >
                −
              </StepButton>
            )}
            <span className="autoterm-modal__power-value">{setpoint}°C</span>
            {!readOnly && (
              <StepButton
                label="Increase temperature"
                disabled={disabled || stepsDisabled || setpoint >= SETPOINT_MAX}
                onClick={() => adjust(setpoint + 1)}
              >
                +
              </StepButton>
            )}
          </div>
        )}
      </div>

      <div className="boiler-modal__footer">
        <div className="boiler-modal__footer-spacer" aria-hidden="true">
          <ModalStatSlots
            items={[
              { label: "Level", value: "100%" },
              { label: "Temp", value: "00.0°C" },
            ]}
          />
        </div>
        <div className="boiler-modal__panel">
          <p
            className="boiler-modal__error"
            role={errorText ? "alert" : undefined}
          >
            {errorText ?? ""}
          </p>
          <div className="boiler-modal__controls">
            <PowerButton
              active={enabled}
              disabled={powerDisabled}
              readOnly={readOnly}
              onClick={onPower}
            />
            <PresetButton
              value={30}
              active={boiler?.mode === "once" && boiler?.once === 30}
              disabled={presetDisabled(30)}
              readOnly={readOnly}
              onClick={() => onOnce?.(30)}
            />
            <PresetButton
              value={60}
              active={boiler?.mode === "once" && boiler?.once === 60}
              disabled={presetDisabled(60)}
              readOnly={readOnly}
              onClick={() => onOnce?.(60)}
            />
          </div>
        </div>
      </div>
    </div>
  );
}

function StepButton({ label, onClick, disabled = false, children }) {
  const lastFireRef = useRef(0);
  const pointerHandledRef = useRef(false);

  const fire = () => {
    if (disabled || !onClick) return;
    const now = Date.now();
    if (now - lastFireRef.current < STEP_BUTTON_DEBOUNCE_MS) {
      return;
    }
    lastFireRef.current = now;
    onClick();
  };

  return (
    <button
      type="button"
      className={`neumorphic-button fridge-ble-modal__round fridge-ble-modal__round--step off${
        disabled ? " fridge-ble-modal__round--inactive" : ""
      }`}
      style={{ touchAction: "manipulation" }}
      onPointerUp={(event) => {
        if (disabled) return;
        if (event.pointerType === "touch" || event.pointerType === "pen") {
          event.preventDefault();
          pointerHandledRef.current = true;
          fire();
        }
      }}
      onClick={() => {
        if (pointerHandledRef.current) {
          pointerHandledRef.current = false;
          return;
        }
        fire();
      }}
      disabled={disabled}
      aria-label={label}
    >
      <span className="button-text fridge-ble-modal__step-glyph">{children}</span>
    </button>
  );
}

function PresetButton({ value, active = false, disabled, readOnly = false, onClick }) {
  const reactId = useId().replace(/:/g, "");
  const gradientId = `boiler-preset-grad-${reactId}`;
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");

  useEffect(() => {
    const updateColors = () => {
      setAccentBlue(getThemeColor("--color-accent-blue") || "#3b82f6");
      setAccentBlueDark(getThemeColor("--color-accent-blue-dark") || "#2563eb");
    };
    updateColors();
    const interval = setInterval(updateColors, 2000);
    return () => clearInterval(interval);
  }, []);

  return (
    <button
      type="button"
      className={`fridge-ble-modal__card${
        !readOnly && disabled ? " fridge-ble-modal__card--disabled" : ""
      }${readOnly ? " boiler-modal__control--static" : ""}`}
      disabled={!readOnly && disabled}
      onClick={readOnly ? undefined : onClick}
      aria-disabled={readOnly ? true : undefined}
      tabIndex={readOnly ? -1 : undefined}
      aria-label={
        readOnly
          ? active
            ? `Heating once to ${value} degrees`
            : `${value} degrees`
          : active
            ? `Stop heating to ${value} degrees`
            : `Heat once to ${value} degrees`
      }
      aria-pressed={active}
    >
      <span
        className={`neumorphic-button fridge-ble-modal__round${
          active ? " on" : " off"
        }`}
      >
        <svg className="horseshoe-progress" viewBox="0 0 200 200" aria-hidden="true">
          <defs>
            <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
              <stop offset="0%" stopColor={accentBlue} />
              <stop offset="100%" stopColor={accentBlueDark} />
            </linearGradient>
          </defs>
          {active && (
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
        <span className="button-text boiler-modal__preset-value">{value}°</span>
      </span>
    </button>
  );
}

function PowerButton({ active, disabled, readOnly = false, onClick }) {
  const reactId = useId().replace(/:/g, "");
  const gradientId = `boiler-btn-grad-${reactId}`;
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");

  useEffect(() => {
    const updateColors = () => {
      setAccentBlue(getThemeColor("--color-accent-blue") || "#3b82f6");
      setAccentBlueDark(getThemeColor("--color-accent-blue-dark") || "#2563eb");
    };
    updateColors();
    const interval = setInterval(updateColors, 2000);
    return () => clearInterval(interval);
  }, []);

  return (
    <button
      type="button"
      className={`fridge-ble-modal__card${
        !readOnly && disabled ? " fridge-ble-modal__card--disabled" : ""
      }${readOnly ? " boiler-modal__control--static" : ""}`}
      disabled={!readOnly && disabled}
      onClick={readOnly ? undefined : onClick}
      aria-disabled={readOnly ? true : undefined}
      tabIndex={readOnly ? -1 : undefined}
      aria-label={
        readOnly
          ? active
            ? "Maintain temperature"
            : "Boiler off"
          : active
            ? "Turn boiler off"
            : "Turn boiler on"
      }
      aria-pressed={active}
    >
      <span
        className={`neumorphic-button fridge-ble-modal__round${
          active ? " on" : " off"
        }`}
      >
        <svg className="horseshoe-progress" viewBox="0 0 200 200" aria-hidden="true">
          <defs>
            <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
              <stop offset="0%" stopColor={accentBlue} />
              <stop offset="100%" stopColor={accentBlueDark} />
            </linearGradient>
          </defs>
          {active && (
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
          <svg
            className="fridge-ble-modal__power-symbol"
            viewBox="0 0 24 24"
            fill="none"
            aria-hidden="true"
          >
            <path
              d="M12 3v8"
              stroke="currentColor"
              strokeWidth="2.25"
              strokeLinecap="round"
            />
            <path
              d="M7.05 6.05a7 7 0 1 0 9.9 0"
              stroke="currentColor"
              strokeWidth="2.25"
              strokeLinecap="round"
            />
          </svg>
        </span>
      </span>
    </button>
  );
}
