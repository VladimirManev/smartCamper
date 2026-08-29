/**
 * Alarm modal — idle: van + lock/cat; exit_delay: countdown + Cancel; else PIN keypad.
 */

import { useCallback, useEffect, useId, useState } from "react";
import { getThemeColor } from "../utils/getThemeColor";
import { CamperDoorsStage, DEFAULT_DOORS } from "./CamperDoorsStage";

const EXIT_COUNTDOWN_START = 29;
const PIN_MAX_LEN = 16;

function LockIcon() {
  return (
    <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <rect
        x="5"
        y="11"
        width="14"
        height="10"
        rx="2"
        stroke="currentColor"
        strokeWidth="2"
      />
      <path
        d="M8 11V8a4 4 0 0 1 8 0v3"
        stroke="currentColor"
        strokeWidth="2"
        strokeLinecap="round"
      />
    </svg>
  );
}

function CatIcon() {
  return (
    <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path
        d="M6 10.5 8.2 5.2c.25-.55.95-.55 1.2 0L11 9.2h2l1.6-4c.25-.55.95-.55 1.2 0L18 10.5"
        stroke="currentColor"
        strokeWidth="2"
        strokeLinecap="round"
        strokeLinejoin="round"
      />
      <path
        d="M6 10.5c-.9 1.1-1.4 2.4-1.4 3.9C4.6 18.2 7.7 20.5 12 20.5s7.4-2.3 7.4-6.1c0-1.5-.5-2.8-1.4-3.9"
        stroke="currentColor"
        strokeWidth="2"
        strokeLinecap="round"
        strokeLinejoin="round"
      />
      <circle cx="9.2" cy="13.2" r="1.15" fill="currentColor" />
      <circle cx="14.8" cy="13.2" r="1.15" fill="currentColor" />
      <path
        d="M12 14.6v1.3M10.6 16.6c.8.55 2 .55 2.8 0"
        stroke="currentColor"
        strokeWidth="1.5"
        strokeLinecap="round"
        strokeLinejoin="round"
      />
      <path
        d="M3.8 12.8h2.6M3.8 14.6h2.4M17.6 12.8h2.6M17.8 14.6h2.4"
        stroke="currentColor"
        strokeWidth="1.5"
        strokeLinecap="round"
      />
    </svg>
  );
}

function ArmButton({
  className = "",
  active,
  disabled,
  gradientId,
  accentBlue,
  accentBlueDark,
  ariaLabel,
  onClick,
  children,
}) {
  return (
    <button
      type="button"
      className={`neumorphic-button perimeter-modal__arm ${active ? "on" : "off"}${
        className ? ` ${className}` : ""
      }`}
      onClick={onClick}
      disabled={disabled}
      aria-pressed={active}
      aria-label={ariaLabel}
    >
      <svg className="horseshoe-progress" viewBox="0 0 200 200" aria-hidden>
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
        <div
          className={`icon-container ${active ? "icon-active" : "icon-inactive"}`}
        >
          {children}
        </div>
      </span>
    </button>
  );
}

const KEYPAD_ROWS = [
  ["1", "2", "3"],
  ["4", "5", "6"],
  ["7", "8", "9"],
  ["*", "0", "#"],
];

/**
 * @param {Object} props
 * @param {boolean} [props.disabled]
 * @param {boolean} [props.armed]
 * @param {string} [props.phase] - idle | exit_delay | armed | entry_delay | alarm
 * @param {boolean} [props.catMode]
 * @param {number|null} [props.exitStartedAt] - epoch ms when exit_delay began (persists across remount)
 * @param {boolean} [props.forceKeypad] - Cancel during exit_delay (persists across remount)
 * @param {Object} [props.doors] - { driver, passenger, sliding, rear }
 * @param {Function} [props.onForceKeypad]
 * @param {Function} [props.onArmNormal]
 * @param {Function} [props.onArmCat]
 * @param {Function} [props.onDisarm] - (pin: string) => void
 */
export function AlarmModalContent({
  disabled = false,
  armed = false,
  phase = "idle",
  catMode = false,
  exitStartedAt = null,
  forceKeypad = false,
  doors = DEFAULT_DOORS,
  onForceKeypad,
  onArmNormal,
  onArmCat,
  onDisarm,
}) {
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");
  const [exitSeconds, setExitSeconds] = useState(EXIT_COUNTDOWN_START);
  const [pin, setPin] = useState("");
  const reactId = useId().replace(/:/g, "");
  const normalGradId = `alarm-arm-normal-${reactId}`;
  const catGradId = `alarm-arm-cat-${reactId}`;

  const normalActive = armed && !catMode && phase !== "idle";
  const catActive = armed && catMode && phase !== "idle";
  const isExitDelay = phase === "exit_delay";
  const showKeypad =
    forceKeypad ||
    phase === "armed" ||
    phase === "entry_delay" ||
    phase === "alarm";
  const showExitUi = isExitDelay && !forceKeypad;
  const showIdleVan = phase === "idle" && !armed;

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
    if (phase === "idle") {
      setPin("");
    }
  }, [phase]);

  // Countdown from exitStartedAt so remounting does not reset to 29
  useEffect(() => {
    if (!isExitDelay || forceKeypad) return undefined;

    const calc = () => {
      if (!exitStartedAt) return EXIT_COUNTDOWN_START;
      return Math.max(
        0,
        EXIT_COUNTDOWN_START - Math.floor((Date.now() - exitStartedAt) / 1000)
      );
    };

    setExitSeconds(calc());
    const id = setInterval(() => setExitSeconds(calc()), 250);
    return () => clearInterval(id);
  }, [isExitDelay, forceKeypad, exitStartedAt]);

  const handleNormal = useCallback(() => {
    if (disabled || !showIdleVan) return;
    onArmNormal?.();
  }, [disabled, onArmNormal, showIdleVan]);

  const handleCat = useCallback(() => {
    if (disabled || !showIdleVan) return;
    onArmCat?.();
  }, [disabled, onArmCat, showIdleVan]);

  const handleCancel = useCallback(() => {
    setPin("");
    onForceKeypad?.();
  }, [onForceKeypad]);

  const handleKey = useCallback(
    (key) => {
      if (disabled) return;
      if (key === "*") {
        setPin("");
        return;
      }
      if (key === "#") {
        if (pin.length === 0) return;
        const submitted = pin;
        setPin("");
        onDisarm?.(submitted);
        return;
      }
      setPin((prev) => (prev.length >= PIN_MAX_LEN ? prev : prev + key));
    },
    [disabled, onDisarm, pin]
  );

  let hint = "Alarm off";
  if (disabled) hint = "Module offline";
  else if (showExitUi) hint = "";
  else if (phase === "entry_delay") hint = "Entry delay · enter PIN";
  else if (phase === "alarm") hint = "ALARM · enter PIN";
  else if (phase === "armed" || forceKeypad) hint = "Enter PIN · # to send · * clear";
  else if (showIdleVan) hint = "Alarm off";

  return (
    <div
      className={`perimeter-modal alarm-modal${
        disabled ? " perimeter-modal--disabled" : ""
      }`}
    >
      {showIdleVan && (
        <div className="perimeter-modal__main">
          <ArmButton
            className="alarm-modal__arm--left"
            active={normalActive}
            disabled={disabled}
            gradientId={normalGradId}
            accentBlue={accentBlue}
            accentBlueDark={accentBlueDark}
            ariaLabel="Arm alarm"
            onClick={handleNormal}
          >
            <LockIcon />
          </ArmButton>

          <ArmButton
            className="alarm-modal__arm--right"
            active={catActive}
            disabled={disabled}
            gradientId={catGradId}
            accentBlue={accentBlue}
            accentBlueDark={accentBlueDark}
            ariaLabel="Arm cat mode"
            onClick={handleCat}
          >
            <CatIcon />
          </ArmButton>

          <CamperDoorsStage
            doors={doors}
            stageClassName="alarm-modal__stage"
            vanLayerClassName="alarm-modal__van-layer"
          />
        </div>
      )}

      {showExitUi && (
        <div className="alarm-modal__exit">
          <div className="alarm-modal__countdown" aria-live="polite">
            {exitSeconds}
          </div>
          <button
            type="button"
            className="alarm-modal__cancel"
            onClick={handleCancel}
            disabled={disabled}
          >
            Cancel
          </button>
        </div>
      )}

      {showKeypad && (
        <div className="alarm-modal__keypad" role="group" aria-label="PIN keypad">
          {KEYPAD_ROWS.map((row) => (
            <div key={row.join("-")} className="alarm-modal__keypad-row">
              {row.map((key) => (
                <button
                  key={key}
                  type="button"
                  className="alarm-modal__key"
                  onClick={() => handleKey(key)}
                  disabled={disabled}
                >
                  {key}
                </button>
              ))}
            </div>
          ))}
        </div>
      )}

      {hint ? <p className="perimeter-modal__hint">{hint}</p> : null}
    </div>
  );
}
