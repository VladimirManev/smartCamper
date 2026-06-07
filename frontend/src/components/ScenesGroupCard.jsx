/**
 * ScenesGroupCard — opens the Scenes preset modal from the main menu.
 */

import { Card } from "./Card";

/**
 * @param {Object} props
 * @param {string} props.name
 * @param {Function} props.onClick
 * @param {Function} [props.onLongPress] - Quick All Off (with confirmation in App)
 */
export function ScenesGroupCard({ name, onClick, onLongPress }) {
  const icon = <i className="fas fa-layer-group" aria-hidden />;

  return (
    <Card
      name={name}
      icon={icon}
      buttonState="off"
      iconState="active"
      onClick={onClick}
      onLongPress={onLongPress}
    />
  );
}
