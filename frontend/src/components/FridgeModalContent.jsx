/**
 * Fridge BLE controls — power (module-5 relay) + mode + zone setpoints.
 *
 * Physical layout: left compartment is colder (BLE zone 2), right is warmer (zone 1).
 * Tap a zone badge to edit setpoint (+/−); idle 3s commits and hides controls.
 * `readOnly` — status carousel: photo + temps only (no controls).
 */

import { useEffect, useId, useRef, useState } from "react";
import fridgePhoto from "../assets/fridge.png";
import { getThemeColor } from "../utils/getThemeColor";

const EDIT_IDLE_MS = 3000;

/**
 * @param {Object} props
 * @param {Object} props.status - from useFridge
 * @param {boolean} props.powerOn - module-5 fridge relay ON
 * @param {Function} [props.onPowerToggle] - toggles module-5 relay
 * @param {boolean} [props.powerDisabled]
 * @param {Function} [props.onMode] - (action: 'eco'|'max'|'toggle') => void
 * @param {Function} [props.onZoneSet] - (zone: 1|2, temp: number) => void
 * @param {boolean} props.disabled - BLE controls disabled (module-6 offline)
 * @param {boolean} [props.readOnly] - status slide: display only
 */
export function FridgeModalContent({
  status,
  powerOn = false,
  onPowerToggle,
  powerDisabled = false,
  onMode,
  onZoneSet,
  disabled = false,
  readOnly = false,
}) {
  const mode = status?.mode;
  const z1 = status?.zone1 || {};
  const z2 = status?.zone2 || {};
  const bleOk = Boolean(status?.connected) && !disabled;

  /** @type {[null|1|2, Function]} BLE zone being edited */
  const [editingZone, setEditingZone] = useState(null);
  const [draftSet, setDraftSet] = useState(null);

  const hideTimerRef = useRef(null);
  const editRef = useRef({ zone: null, draft: null });
  const onZoneSetRef = useRef(onZoneSet);

  useEffect(() => {
    editRef.current = { zone: editingZone, draft: draftSet };
  }, [editingZone, draftSet]);

  useEffect(() => {
    onZoneSetRef.current = onZoneSet;
  }, [onZoneSet]);

  useEffect(() => {
    return () => {
      if (hideTimerRef.current) {
        clearTimeout(hideTimerRef.current);
      }
    };
  }, []);

  const clearHideTimer = () => {
    if (hideTimerRef.current) {
      clearTimeout(hideTimerRef.current);
      hideTimerRef.current = null;
    }
  };

  const commitAndHide = () => {
    clearHideTimer();
    const { zone, draft } = editRef.current;
    if (zone != null && draft !== null && onZoneSetRef.current) {
      onZoneSetRef.current(zone, draft);
    }
    setEditingZone(null);
    setDraftSet(null);
  };

  const scheduleHide = () => {
    clearHideTimer();
    hideTimerRef.current = setTimeout(commitAndHide, EDIT_IDLE_MS);
  };

  const fridgeOff = !powerOn;

  useEffect(() => {
    if (!fridgeOff || editingZone == null) {
      return;
    }
    clearHideTimer();
    setEditingZone(null);
    setDraftSet(null);
  }, [fridgeOff, editingZone]);

  const fmtTemp = (v) =>
    v === null || v === undefined || Number.isNaN(v)
      ? "—"
      : `${Math.round(v)}°C`;

  const openEditor = (zone, setpoint) => {
    if (readOnly || fridgeOff || !bleOk) {
      return;
    }
    if (
      editingZone != null &&
      editingZone !== zone &&
      draftSet !== null
    ) {
      onZoneSet?.(editingZone, draftSet);
    }
    const start =
      editingZone === zone && draftSet !== null
        ? draftSet
        : setpoint === null || setpoint === undefined || Number.isNaN(setpoint)
          ? 0
          : Math.round(setpoint);
    setEditingZone(zone);
    setDraftSet(start);
    scheduleHide();
  };

  const bumpDraft = (delta) => {
    if (draftSet === null) {
      return;
    }
    setDraftSet((v) => Math.max(-20, Math.min(20, v + delta)));
    scheduleHide();
  };

  const setLabelFor = (zone, liveSetpoint) => {
    if (editingZone === zone && draftSet !== null) {
      return fmtSet(draftSet);
    }
    return fmtSet(liveSetpoint);
  };

  const displayTemp = (v) => (fridgeOff ? "OFF" : fmtTemp(v));
  const canEditZones = !readOnly && !fridgeOff && bleOk;
  const canChangeMode = !fridgeOff && bleOk;

  const renderBadge = (side, zone, zoneData, flakeClass) => {
    const className = `fridge-ble-modal__badge fridge-ble-modal__badge--${side}${
      !readOnly && editingZone === zone ? " fridge-ble-modal__badge--editing" : ""
    }${!canEditZones ? " fridge-ble-modal__badge--disabled" : ""}${
      readOnly ? " fridge-ble-modal__badge--static" : ""
    }`;
    const label = fridgeOff
      ? `${side === "left" ? "Left" : "Right"} zone OFF`
      : `${side === "left" ? "Left" : "Right"} zone ${fmtTemp(
          zoneData.temp
        )}, set ${setLabelFor(zone, zoneData.setpoint)}`;
    const content = (
      <>
        <SnowflakeIcon className={`fridge-ble-modal__flake ${flakeClass}`} />
        <span className="fridge-ble-modal__badge-text">
          <span className="fridge-ble-modal__badge-temp">
            {displayTemp(zoneData.temp)}
          </span>
          {!fridgeOff && (
            <span className="fridge-ble-modal__badge-set">
              (set {setLabelFor(zone, zoneData.setpoint)})
            </span>
          )}
        </span>
      </>
    );

    if (readOnly || fridgeOff) {
      return (
        <div className={className} aria-label={label}>
          {content}
        </div>
      );
    }

    return (
      <button
        type="button"
        className={className}
        disabled={!canEditZones}
        onClick={() => openEditor(zone, zoneData.setpoint)}
        aria-label={`${label}. Tap to adjust`}
      >
        {content}
      </button>
    );
  };

  return (
    <div
      className={`fridge-ble-modal${
        readOnly ? " fridge-ble-modal--readonly" : ""
      }`}
    >
      <div className="fridge-ble-modal__photo-wrap">
        <img
          className="fridge-ble-modal__photo"
          src={fridgePhoto}
          alt="AAOBOSI dual-zone fridge"
          draggable={false}
        />

        {renderBadge("left", 2, z2, "fridge-ble-modal__flake--blue")}
        {renderBadge("right", 1, z1, "fridge-ble-modal__flake--green")}

        {!readOnly && !fridgeOff && editingZone != null && (
          <div className="fridge-ble-modal__zone-steps">
            <StepButton label="Colder" onClick={() => bumpDraft(-1)}>
              −
            </StepButton>
            <StepButton label="Warmer" onClick={() => bumpDraft(1)}>
              +
            </StepButton>
          </div>
        )}
      </div>

      {!readOnly && (
        <div className="fridge-ble-modal__actions">
          <FridgeCardButton
            label={powerOn ? "ON" : "OFF"}
            active={powerOn}
            disabled={powerDisabled}
            onClick={onPowerToggle}
            ariaLabel={powerOn ? "Turn fridge off" : "Turn fridge on"}
          />

          <div className="fridge-ble-modal__mode-group">
            <FridgeCardButton
              label="MAX"
              active={mode === "max"}
              disabled={!canChangeMode}
              onClick={() => onMode?.("max")}
              ariaLabel="MAX mode"
            />
            <FridgeCardButton
              label="ECO"
              active={mode === "eco"}
              disabled={!canChangeMode}
              onClick={() => onMode?.("eco")}
              ariaLabel="ECO mode"
            />
          </div>
        </div>
      )}
    </div>
  );
}

function FridgeCardButton({ label, active, disabled, onClick, ariaLabel }) {
  const reactId = useId().replace(/:/g, "");
  const gradientId = `fridge-btn-grad-${reactId}`;
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
        disabled ? " fridge-ble-modal__card--disabled" : ""
      }`}
      disabled={disabled}
      onClick={onClick}
      aria-label={ariaLabel}
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
        <span className="button-text">{label}</span>
      </span>
    </button>
  );
}

function fmtSet(v) {
  return v === null || v === undefined || Number.isNaN(v)
    ? "—"
    : `${Math.round(v)}°`;
}

function StepButton({ label, onClick, children }) {
  return (
    <button
      type="button"
      className="neumorphic-button fridge-ble-modal__round fridge-ble-modal__round--step off"
      onClick={onClick}
      aria-label={label}
    >
      <span className="button-text fridge-ble-modal__step-glyph">{children}</span>
    </button>
  );
}

function SnowflakeIcon({ className }) {
  return (
    <svg
      className={className}
      viewBox="0 0 24 24"
      fill="none"
      xmlns="http://www.w3.org/2000/svg"
      aria-hidden="true"
    >
      <path
        d="M12 2v20M4.5 6.5l15 11M19.5 6.5l-15 11M3 12h18"
        stroke="currentColor"
        strokeWidth="2"
        strokeLinecap="round"
      />
      <path
        d="M12 5l-1.6-1.6M12 5l1.6-1.6M12 19l-1.6 1.6M12 19l1.6 1.6M5.2 8.2l-2.1.3M5.2 8.2l.3-2.1M18.8 8.2l2.1.3M18.8 8.2l-.3-2.1M5.2 15.8l-2.1-.3M5.2 15.8l.3 2.1M18.8 15.8l2.1-.3M18.8 15.8l-.3 2.1"
        stroke="currentColor"
        strokeWidth="1.6"
        strokeLinecap="round"
      />
    </svg>
  );
}
