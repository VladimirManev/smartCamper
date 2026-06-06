/**
 * Compact peripheral node on the battery energy diagram.
 */

function SolarPanelIcon() {
  const bl = { x: 3.5, y: 15.5 };
  const br = { x: 19.5, y: 15.5 };
  const tr = { x: 20.5, y: 5.5 };
  const tl = { x: 4.5, y: 8.5 };

  const lerp = (a, b, t) => ({
    x: a.x + (b.x - a.x) * t,
    y: a.y + (b.y - a.y) * t,
  });

  const outline = `M ${bl.x} ${bl.y} L ${br.x} ${br.y} L ${tr.x} ${tr.y} L ${tl.x} ${tl.y} Z`;

  const gridH = [];
  const gridV = [];
  for (let i = 1; i < 4; i += 1) {
    const t = i / 4;
    const left = lerp(bl, tl, t);
    const right = lerp(br, tr, t);
    gridH.push(`M ${left.x} ${left.y} L ${right.x} ${right.y}`);
    const bottom = lerp(bl, br, t);
    const top = lerp(tl, tr, t);
    gridV.push(`M ${bottom.x} ${bottom.y} L ${top.x} ${top.y}`);
  }

  const mountX = (bl.x + br.x) / 2;
  const mountY = bl.y;

  return (
    <svg
      className="battery-node-card__diagram-icon"
      viewBox="0 0 24 24"
      aria-hidden="true"
    >
      <path
        d={outline}
        fill="none"
        stroke="currentColor"
        strokeWidth="1.1"
        strokeLinejoin="round"
      />
      <g fill="none" stroke="currentColor" strokeWidth="0.85" strokeLinecap="round">
        {gridH.map((d, i) => (
          <path key={`h-${i}`} d={d} />
        ))}
        {gridV.map((d, i) => (
          <path key={`v-${i}`} d={d} />
        ))}
      </g>
      <g stroke="currentColor" strokeWidth="1.1" strokeLinecap="round" strokeLinejoin="round">
        <line x1={mountX} y1={mountY} x2={mountX} y2={21.2} />
        <line x1={mountX - 2.8} y1={21.2} x2={mountX} y2={19.8} />
        <line x1={mountX + 2.8} y1={21.2} x2={mountX} y2={19.8} />
        <line x1={mountX - 3.2} y1={21.2} x2={mountX + 3.2} y2={21.2} />
      </g>
    </svg>
  );
}

function AlternatorIcon() {
  const brushRect = (x, y) => (
    <rect
      x={x}
      y={y}
      width="2"
      height="4.2"
      rx="0.35"
      fill="none"
      stroke="currentColor"
      strokeWidth="1.1"
    />
  );

  return (
    <svg
      className="battery-node-card__diagram-icon battery-node-card__diagram-icon--alternator"
      viewBox="0 0 24 24"
      aria-hidden="true"
    >
      {brushRect(2.2, 9.9)}
      {brushRect(19.8, 9.9)}
      <circle
        cx="12"
        cy="12"
        r="5.5"
        fill="none"
        stroke="currentColor"
        strokeWidth="1.1"
      />
      <circle
        cx="12"
        cy="12"
        r="3.9"
        fill="none"
        stroke="currentColor"
        strokeWidth="0.85"
        opacity="0.7"
      />
      <circle
        cx="12"
        cy="12"
        r="1.75"
        fill="none"
        stroke="currentColor"
        strokeWidth="1.1"
      />
    </svg>
  );
}

function DcLoadsIcon() {
  return (
    <svg
      className="battery-node-card__diagram-icon battery-node-card__diagram-icon--out"
      viewBox="0 0 24 24"
      aria-hidden="true"
    >
      <rect
        x="3.5"
        y="5.5"
        width="17"
        height="13"
        rx="1.8"
        fill="none"
        stroke="currentColor"
        strokeWidth="1.1"
      />
      <text
        className="battery-node-card__diagram-text battery-node-card__diagram-text--out"
        x="12"
        y="12.4"
        textAnchor="middle"
        dominantBaseline="middle"
      >
        OUT
      </text>
    </svg>
  );
}

function BatteryNodeDevice({ label }) {
  const sidePins = [0, 1, 2];
  const bottomPins = [0, 1, 2, 3, 4];

  return (
    <div className="battery-node-card__device">
      <div className="battery-node-card__device-row">
        <div
          className="battery-node-card__device-pins battery-node-card__device-pins--side"
          aria-hidden="true"
        >
          {sidePins.map((i) => (
            <span key={`l-${i}`} className="battery-node-card__device-pin" />
          ))}
        </div>
        <div className="battery-node-card__device-body">
          <span className="battery-node-card__device-text">{label}</span>
        </div>
        <div
          className="battery-node-card__device-pins battery-node-card__device-pins--side"
          aria-hidden="true"
        >
          {sidePins.map((i) => (
            <span key={`r-${i}`} className="battery-node-card__device-pin" />
          ))}
        </div>
      </div>
      <div
        className="battery-node-card__device-pins battery-node-card__device-pins--bottom"
        aria-hidden="true"
      >
        {bottomPins.map((i) => (
          <span key={`b-${i}`} className="battery-node-card__device-pin" />
        ))}
      </div>
    </div>
  );
}

/**
 * @param {Object} props
 * @param {string} props.label
 * @param {string} props.icon - Font Awesome class without style prefix (e.g. "fa-sun")
 * @param {string} [props.image] - optional image URL (public path)
 * @param {string} [props.deviceLabel] - short label on device-style graphic (e.g. MPPT)
 * @param {boolean} [props.largeIcon]
 * @param {'solar' | 'alternator' | 'dc-loads'} [props.iconKind]
 * @param {'outline'} [props.iconStyle]
 * @param {boolean} [props.dataOffline] - no fresh Victron signal (stale or never seen)
 * @param {boolean} props.disabled
 */
export function BatteryNodeCard({
  label,
  icon,
  image,
  deviceLabel,
  iconKind,
  largeIcon = false,
  iconStyle,
  dataOffline = false,
  disabled = false,
}) {
  const hasDiagramIcon = iconKind === "solar" || iconKind === "alternator" || iconKind === "dc-loads";

  const iconClass = [
    "battery-node-card__icon",
    image && "battery-node-card__icon--image",
    (largeIcon || deviceLabel || hasDiagramIcon) && "battery-node-card__icon--large",
    iconStyle === "outline" && "battery-node-card__icon--outline",
  ]
    .filter(Boolean)
    .join(" ");

  return (
    <div
      className={[
        "battery-node-card",
        disabled && "battery-node-card--disabled",
        dataOffline && "battery-node-card--offline",
      ]
        .filter(Boolean)
        .join(" ")}
    >
      {dataOffline && !disabled && (
        <span className="battery-node-card__off-badge" aria-label={`${label} no live data`}>
          OFF
        </span>
      )}
      <div className={iconClass} aria-hidden="true">
        {image ? (
          <img
            key={image}
            src={image}
            alt=""
            className="battery-node-card__icon-img"
          />
        ) : deviceLabel ? (
          <BatteryNodeDevice label={deviceLabel} />
        ) : iconKind === "solar" ? (
          <SolarPanelIcon />
        ) : iconKind === "alternator" ? (
          <AlternatorIcon />
        ) : iconKind === "dc-loads" ? (
          <DcLoadsIcon />
        ) : (
          <i className={`fa-solid ${icon}`} />
        )}
      </div>
      <div className="battery-node-card__label">{label}</div>
    </div>
  );
}
