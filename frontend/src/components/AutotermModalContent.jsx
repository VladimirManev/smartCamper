/**
 * Autoterm heater modal:
 * - Status + card-style power button → Mode (Manual | Comfort | Eco)
 * - Manual: power 1–9 + damper presets/cards (former Airflow)
 * - Comfort / Eco: target temp + read-only power bars + read-only dampers
 * - Panel vent/thermostat lock: stop only
 */

import { useEffect, useRef, useState } from "react";
import { CustomDropdown } from "./CustomDropdown";
import { DamperCard } from "./DamperCard";

const STEP_BUTTON_DEBOUNCE_MS = 120;

export const AUTOTERM_APP_MODES = [
  { value: "manual", label: "Manual" },
  { value: "comfort", label: "Comfort" },
  { value: "eco", label: "Eco" },
];

const DEFAULT_TARGET_TEMP = 20;
const MIN_TARGET_TEMP = 5;
const MAX_TARGET_TEMP = 30;

const DAMPER_NAMES = ["Front", "Rear", "Bath", "Shoes", "Cockpit"];

function formatHeaterStatus(state) {
  switch (state) {
    case "off":
      return "Off";
    case "starting":
      return "Starting…";
    case "running":
    case "running?":
      return "Heating";
    case "vent":
      return "Ventilation";
    case "shutdown":
      return "Shutting down…";
    default:
      return "Unknown";
  }
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
      onPointerUp={(e) => {
        if (disabled) return;
        // Touch/pen: act on pointerup and ignore the synthetic click that follows
        if (e.pointerType === "touch" || e.pointerType === "pen") {
          e.preventDefault();
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

/** Autoterm-style power bars: 9 ascending ticks, filled up to `level`. */
function PowerLevelBars({ level = 1 }) {
  const clamped = Math.min(9, Math.max(1, Number(level) || 1));
  return (
    <div
      className="autoterm-power-bars"
      role="img"
      aria-label={`Power level ${clamped} of 9`}
    >
      {Array.from({ length: 9 }, (_, i) => {
        const n = i + 1;
        const filled = n <= clamped;
        return (
          <span
            key={n}
            className={`autoterm-power-bars__bar${
              filled ? " autoterm-power-bars__bar--filled" : ""
            }`}
            style={{ "--bar-index": i }}
          />
        );
      })}
    </div>
  );
}

function PowerStepper({
  level,
  onDecrease,
  onIncrease,
  canEdit,
  showControls = true,
  decreaseLabel = "Decrease power",
  increaseLabel = "Increase power",
}) {
  if (!showControls) {
    return (
      <div className="autoterm-modal__stepper autoterm-modal__stepper--bars-only">
        <PowerLevelBars level={level} />
      </div>
    );
  }

  return (
    <div className="autoterm-modal__stepper">
      <StepButton
        label={decreaseLabel}
        disabled={!canEdit || level <= 1}
        onClick={onDecrease}
      >
        −
      </StepButton>
      <PowerLevelBars level={level} />
      <StepButton
        label={increaseLabel}
        disabled={!canEdit || level >= 9}
        onClick={onIncrease}
      >
        +
      </StepButton>
    </div>
  );
}

function DamperGrid({
  dampers,
  disabled = false,
  readOnly = false,
  onDamperToggle,
  onDamperLongPress,
}) {
  return (
    <div
      className={`modal-grid${readOnly ? " autoterm-modal__dampers--readonly" : ""}`}
      aria-disabled={readOnly || undefined}
    >
      {DAMPER_NAMES.map((name, index) => (
        <div className="card-wrapper" key={name}>
          <DamperCard
            name={name}
            damper={dampers[index]}
            onClick={
              readOnly || disabled ? undefined : () => onDamperToggle?.(index)
            }
            onLongPress={
              readOnly || disabled
                ? undefined
                : () => onDamperLongPress?.(index, name, dampers[index])
            }
            disabled={disabled && !readOnly}
          />
          <p className="card-label">{name}</p>
        </div>
      ))}
    </div>
  );
}

export function AutotermModalContent({
  status,
  disabled = false,
  onStart,
  onStop,
  onSetPower,
  dampers = {},
  damperPresets = [],
  selectedPreset = "Manual",
  onPresetChange,
  onDamperToggle,
  onDamperLongPress,
}) {
  const state = status?.state || "unknown";
  const heaterMode = status?.mode || "unknown";
  const statusPower = status?.power ?? 5;
  const locked = !!status?.controlLocked;
  const lockReason = status?.lockReason;
  const isOff = state === "off";
  const isOn =
    state === "running" ||
    state === "starting" ||
    state === "vent" ||
    state === "shutdown";

  const [appMode, setAppMode] = useState("manual");
  const [draftPower, setDraftPower] = useState(statusPower);
  const [targetTemp, setTargetTemp] = useState(DEFAULT_TARGET_TEMP);

  useEffect(() => {
    setDraftPower(statusPower);
  }, [statusPower]);

  const errorCode = status?.errorCode || 0;
  const hasError = errorCode > 0;

  const lockMessage =
    lockReason === "vent"
      ? "Ventilation was started from the original panel — control it there. You can only turn it off here."
      : "A thermostat mode was started from the original panel — control it there. You can only turn it off here.";

  const canEditPower =
    !disabled && !locked && (isOff || heaterMode === "by-power");
  const displayPower = isOff ? draftPower : statusPower;
  const statusLabel = formatHeaterStatus(state);
  const heaterTemp =
    typeof status?.heaterTemp === "number" ? status.heaterTemp : null;
  const heaterTempLabel =
    heaterTemp != null ? `${heaterTemp}°C` : "—";

  const adjustPower = (next) => {
    if (next < 1 || next > 9 || !canEditPower) {
      return;
    }
    if (isOff) {
      setDraftPower(next);
      return;
    }
    onSetPower?.(next);
  };

  const adjustTargetTemp = (next) => {
    if (next < MIN_TARGET_TEMP || next > MAX_TARGET_TEMP || disabled) {
      return;
    }
    setTargetTemp(next);
  };

  const presetOptions = [
    { value: "Manual", label: "Manual" },
    ...damperPresets.map((preset) => ({
      value: preset.name,
      label: preset.name,
    })),
  ];

  const handlePowerToggle = () => {
    if (disabled) return;
    if (locked || !isOff) {
      onStop?.();
      return;
    }
    onStart?.(draftPower);
  };

  const powerButtonState = hasError
    ? "error"
    : locked
      ? "temp-control-off"
      : isOn
        ? "on"
        : "off";
  const powerIconOn = isOn || locked;

  const powerToggleButton = (
    <button
      type="button"
      className={`neumorphic-button autoterm-modal__power-btn ${powerButtonState}`}
      disabled={disabled}
      onClick={handlePowerToggle}
      aria-label={isOff && !locked ? "Turn on" : "Turn off"}
    >
      {powerIconOn && (
        <svg className="horseshoe-progress" viewBox="0 0 200 200" aria-hidden="true">
          <circle
            className="horseshoe-fill"
            cx="100"
            cy="100"
            r="80"
            fill="none"
            stroke="var(--color-accent-blue)"
            strokeWidth="8"
            strokeLinecap="round"
          />
        </svg>
      )}
      <span className="button-text autoterm-modal__power-btn-label">
        {hasError ? (
          "!"
        ) : (
          <svg
            className="autoterm-modal__power-symbol"
            viewBox="0 0 24 24"
            fill="none"
            aria-hidden="true"
          >
            {/* Remote-style power: vertical stem + 3/4 arc (⏻) */}
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
        )}
      </span>
    </button>
  );

  return (
    <div className="autoterm-modal">
      {hasError && (
        <div className="autoterm-modal__error" role="alert">
          <strong>Error {errorCode}</strong>
          <span>{status?.error || "Unknown heater fault"}</span>
        </div>
      )}

      <div className="autoterm-modal__live-status" role="status" aria-live="polite">
        {powerToggleButton}
        <div className="autoterm-modal__live-status-text settings-label">
          <p className="autoterm-modal__live-status-line">
            Status:{" "}
            <span className="autoterm-modal__live-status-value">{statusLabel}</span>
          </p>
          <p className="autoterm-modal__live-status-line">
            Heater temp:{" "}
            <span className="autoterm-modal__live-status-value">
              {heaterTempLabel}
            </span>
          </p>
        </div>
      </div>

      {locked ? (
        <div className="autoterm-modal__locked">
          <p>{lockMessage}</p>
          <p className="autoterm-modal__hint">Stop only — use the button above.</p>
        </div>
      ) : (
        <>
          <div className="settings-row">
            <label className="settings-label">Mode</label>
            <CustomDropdown
              value={appMode}
              onChange={setAppMode}
              options={AUTOTERM_APP_MODES}
              disabled={disabled}
              className="settings-dropdown"
            />
          </div>

          {appMode === "manual" && (
            <div className="autoterm-modal__manual">
              <div className="autoterm-modal__power-block">
                <PowerStepper
                  level={displayPower}
                  canEdit={canEditPower}
                  onDecrease={() => adjustPower(displayPower - 1)}
                  onIncrease={() => adjustPower(displayPower + 1)}
                />
              </div>

              <div className="settings-row">
                <label className="settings-label">Airflow</label>
                <CustomDropdown
                  value={selectedPreset}
                  onChange={(presetName) => {
                    onPresetChange?.(presetName);
                  }}
                  options={presetOptions}
                  placeholder="Manual"
                  disabled={disabled}
                  className="settings-dropdown"
                  menuPlacement="top"
                />
              </div>

              <DamperGrid
                dampers={dampers}
                disabled={disabled}
                onDamperToggle={onDamperToggle}
                onDamperLongPress={onDamperLongPress}
              />
            </div>
          )}

          {(appMode === "comfort" || appMode === "eco") && (
            <div className="autoterm-modal__temp">
              <div className="autoterm-modal__stepper">
                <StepButton
                  label="Decrease temperature"
                  disabled={disabled || targetTemp <= MIN_TARGET_TEMP}
                  onClick={() => adjustTargetTemp(targetTemp - 1)}
                >
                  −
                </StepButton>
                <span className="autoterm-modal__power-value">
                  {targetTemp}°C
                </span>
                <StepButton
                  label="Increase temperature"
                  disabled={disabled || targetTemp >= MAX_TARGET_TEMP}
                  onClick={() => adjustTargetTemp(targetTemp + 1)}
                >
                  +
                </StepButton>
              </div>

              <PowerStepper level={statusPower} canEdit={false} showControls={false} />

              <DamperGrid dampers={dampers} readOnly />
            </div>
          )}
        </>
      )}
    </div>
  );
}
