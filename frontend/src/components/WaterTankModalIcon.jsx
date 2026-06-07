/**
 * Icon shown in water tank detail modals (matches main-menu card markers).
 * @param {"gray"|"fresh"|"toilet"} variant
 */
export function WaterTankModalIcon({ variant }) {
  const className = "gray-water-modal-icon water-tank-card-icon";

  if (variant === "gray") {
    return <i className={`fas fa-droplet ${className}`} aria-hidden="true" />;
  }

  if (variant === "fresh") {
    return <i className={`fas fa-faucet ${className}`} aria-hidden="true" />;
  }

  return (
    <span
      className={`${className} gray-water-modal-icon--label`}
      aria-hidden="true"
    >
      WC
    </span>
  );
}
