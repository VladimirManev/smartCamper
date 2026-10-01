/**
 * Fridge BLE controls — power (module-5 relay) + mode + zone setpoints.
 *
 * Physical layout: left compartment is colder (BLE zone 2), right is warmer (zone 1).
 */

/**
 * @param {Object} props
 * @param {Object} props.status - from useFridge
 * @param {boolean} props.powerOn - module-5 fridge relay ON
 * @param {Function} props.onPowerToggle - toggles module-5 relay
 * @param {boolean} [props.powerDisabled]
 * @param {Function} props.onMode - (action: 'eco'|'max'|'toggle') => void
 * @param {Function} props.onZoneSet - (zone: 1|2, temp: number) => void
 * @param {boolean} props.disabled - BLE controls disabled (module-6 offline)
 */
export function FridgeModalContent({
  status,
  powerOn = false,
  onPowerToggle,
  powerDisabled = false,
  onMode,
  onZoneSet,
  disabled = false,
}) {
  const mode = status?.mode;
  const z1 = status?.zone1 || {};
  const z2 = status?.zone2 || {};
  const bleOk = Boolean(status?.connected) && !disabled;

  const fmt = (v) =>
    v === null || v === undefined || Number.isNaN(v) ? "—" : `${Math.round(v)}°`;

  const bump = (zone, setpoint, delta) => {
    if (!bleOk || setpoint === null || setpoint === undefined) {
      return;
    }
    const next = Math.max(-20, Math.min(20, Math.round(setpoint) + delta));
    onZoneSet(zone, next);
  };

  return (
    <div className="fridge-ble-modal">
      <button
        type="button"
        className={`fridge-ble-modal__power${
          powerOn ? " fridge-ble-modal__power--on" : ""
        }`}
        disabled={powerDisabled}
        onClick={onPowerToggle}
      >
        {powerOn ? "ON" : "OFF"}
      </button>

      <div
        className={`fridge-ble-modal__controls${
          bleOk ? "" : " fridge-ble-modal__controls--inactive"
        }`}
      >
      <div className="fridge-ble-modal__mode">
        <button
          type="button"
          className={`fridge-ble-modal__mode-btn${
            mode === "max" ? " fridge-ble-modal__mode-btn--active" : ""
          }`}
          disabled={!bleOk}
          onClick={() => onMode("max")}
        >
          MAX
        </button>
        <button
          type="button"
          className={`fridge-ble-modal__mode-btn${
            mode === "eco" ? " fridge-ble-modal__mode-btn--active" : ""
          }`}
          disabled={!bleOk}
          onClick={() => onMode("eco")}
        >
          ECO
        </button>
      </div>

      <div className="fridge-ble-modal__zones">
        <ZoneColumn
          label="Left"
          temp={z2.temp}
          setpoint={z2.setpoint}
          fmt={fmt}
          disabled={!bleOk}
          onMinus={() => bump(2, z2.setpoint, -1)}
          onPlus={() => bump(2, z2.setpoint, 1)}
        />
        <ZoneColumn
          label="Right"
          temp={z1.temp}
          setpoint={z1.setpoint}
          fmt={fmt}
          disabled={!bleOk}
          onMinus={() => bump(1, z1.setpoint, -1)}
          onPlus={() => bump(1, z1.setpoint, 1)}
        />
      </div>
      </div>
    </div>
  );
}

function ZoneColumn({ label, temp, setpoint, fmt, disabled, onMinus, onPlus }) {
  return (
    <div className="fridge-ble-modal__zone">
      <span className="fridge-ble-modal__zone-label">{label}</span>
      <span className="fridge-ble-modal__zone-temp">{fmt(temp)}</span>
      <span className="fridge-ble-modal__zone-set">set {fmt(setpoint)}</span>
      <div className="fridge-ble-modal__zone-btns">
        <button
          type="button"
          className="fridge-ble-modal__step"
          disabled={disabled}
          onClick={onMinus}
          aria-label={`${label} colder`}
        >
          <span className="fridge-ble-modal__step-glyph" aria-hidden="true">
            −
          </span>
        </button>
        <button
          type="button"
          className="fridge-ble-modal__step"
          disabled={disabled}
          onClick={onPlus}
          aria-label={`${label} warmer`}
        >
          <span className="fridge-ble-modal__step-glyph" aria-hidden="true">
            +
          </span>
        </button>
      </div>
    </div>
  );
}
