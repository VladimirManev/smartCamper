/**
 * AlarmCard — main-menu entry for the security alarm panel.
 */

import { Card } from "./Card";

/**
 * @param {Object} props
 * @param {string} props.name
 * @param {Function} props.onClick
 * @param {Function} [props.onLongPress]
 * @param {boolean} [props.disabled]
 */
export function AlarmCard({
  name,
  onClick,
  onLongPress,
  disabled = false,
}) {
  const icon = <i className="fas fa-shield-halved" aria-hidden />;

  return (
    <Card
      name={name}
      icon={icon}
      buttonState="off"
      iconState={disabled ? "gray" : "active"}
      onClick={onClick}
      onLongPress={onLongPress}
      disabled={disabled}
    />
  );
}
