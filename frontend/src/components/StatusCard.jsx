/**
 * StatusCard — tablet main menu; opens rotating status panel in the side modal.
 */

import { Card } from "./Card";

/**
 * @param {Object} props
 * @param {string} props.name
 * @param {Function} props.onClick
 * @param {boolean} [props.disabled]
 */
export function StatusCard({ name, onClick, disabled = false }) {
  const icon = <i className="fas fa-gauge-high" aria-hidden />;

  return (
    <Card
      name={name}
      icon={icon}
      buttonState="off"
      iconState="active"
      onClick={onClick}
      disabled={disabled}
    />
  );
}
