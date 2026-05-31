/**
 * Compact peripheral node on the battery energy diagram.
 */

/**
 * @param {Object} props
 * @param {string} props.label
 * @param {string} props.icon - Font Awesome class without "fa-solid" prefix (e.g. "fa-sun")
 * @param {string} [props.image] - optional image URL (public path)
 * @param {boolean} props.disabled
 */
export function BatteryNodeCard({
  label,
  icon,
  image,
  disabled = false,
}) {
  return (
    <div className={`battery-node-card ${disabled ? "battery-node-card--disabled" : ""}`}>
      <div
        className={`battery-node-card__icon ${image ? "battery-node-card__icon--image" : ""}`}
        aria-hidden="true"
      >
        {image ? (
          <img
            key={image}
            src={image}
            alt=""
            className="battery-node-card__icon-img"
          />
        ) : (
          <i className={`fa-solid ${icon}`} />
        )}
      </div>
      <div className="battery-node-card__label">{label}</div>
    </div>
  );
}
