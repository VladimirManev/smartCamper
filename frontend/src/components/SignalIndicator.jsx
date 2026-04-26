/**
 * SignalIndicator — WiFi fan (two arcs) + dot under apex + module digit in corner
 */

/**
 * @param {number|null} rssi - WiFi RSSI in dBm
 * @returns {number} Bars 0–4
 */
const getSignalBars = (rssi) => {
  if (rssi === null || rssi === undefined || rssi === -999) {
    return 0;
  }
  if (rssi >= -50) return 4;
  if (rssi >= -60) return 3;
  if (rssi >= -70) return 2;
  if (rssi >= -80) return 1;
  return 0;
};

/** 0 = none, 1 = inner arc only, 2 = both arcs */
const getArcTier = (bars) => (bars <= 0 ? 0 : Math.min(2, bars));

const CX = 12;
const CY = 21.85;
const ANGLE0 = (144 * Math.PI) / 180;
const ANGLE1 = (36 * Math.PI) / 180;

function arcPath(r) {
  const sx = CX + r * Math.cos(ANGLE0);
  const sy = CY - r * Math.sin(ANGLE0);
  const ex = CX + r * Math.cos(ANGLE1);
  const ey = CY - r * Math.sin(ANGLE1);
  return `M${sx.toFixed(2)} ${sy.toFixed(2)}A${r} ${r} 0 0 1 ${ex.toFixed(2)} ${ey.toFixed(2)}`;
}

const ARC_PATHS = {
  outer: arcPath(9),
  inner: arcPath(4.1),
};

/**
 * @param {Object} props
 * @param {string} props.moduleNumber
 * @param {boolean} props.isOnline
 * @param {number|null} props.rssi
 * @param {string} props.label
 */
export const SignalIndicator = ({ moduleNumber, isOnline, rssi, label }) => {
  const bars = isOnline ? getSignalBars(rssi) : 0;
  const arcTier = isOnline ? getArcTier(bars) : 0;
  const colorClass = isOnline ? "online" : "offline";
  const title =
    label ||
    `Module ${moduleNumber} — ${
      isOnline && rssi !== null && rssi !== undefined && rssi !== -999
        ? `${rssi} dBm`
        : isOnline
          ? "No RSSI"
          : "Offline"
    }`;

  return (
    <div className={`signal-indicator ${colorClass}`} title={title}>
      <div className="signal-indicator__icon-wrap">
        <svg className="signal-wifi-svg" viewBox="0 0 24 24" aria-hidden>
          <path
            className={`signal-wifi-arc ${arcTier >= 2 ? "is-active" : ""}`}
            d={ARC_PATHS.outer}
            fill="none"
            strokeWidth="1.9"
            strokeLinecap="round"
            strokeLinejoin="round"
          />
          <path
            className={`signal-wifi-arc ${arcTier >= 1 ? "is-active" : ""}`}
            d={ARC_PATHS.inner}
            fill="none"
            strokeWidth="1.9"
            strokeLinecap="round"
            strokeLinejoin="round"
          />
          <circle
            className={`signal-wifi-dot ${isOnline ? "is-active" : ""}`}
            cx="12"
            cy="22.05"
            r="1.08"
          />
        </svg>
        <span className="signal-module-number">{moduleNumber}</span>
      </div>
    </div>
  );
};
