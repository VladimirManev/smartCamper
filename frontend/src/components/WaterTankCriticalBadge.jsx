/**
 * iOS-style alert badge for critical water levels (top-right of card).
 */

/**
 * @param {Object} props
 * @param {boolean} props.show
 */
export function WaterTankCriticalBadge({ show }) {
  if (!show) return null;

  return (
    <span className="water-tank-critical-badge" aria-label="Critical level">
      !
    </span>
  );
}
