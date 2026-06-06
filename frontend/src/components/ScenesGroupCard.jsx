/**
 * ScenesGroupCard — opens the Scenes preset modal from the main menu.
 */

import { Card } from "./Card";

/**
 * @param {Object} props
 * @param {string} props.name
 * @param {Function} props.onClick
 */
export function ScenesGroupCard({ name, onClick }) {
  const icon = <i className="fas fa-layer-group" aria-hidden />;

  return (
    <Card
      name={name}
      icon={icon}
      buttonState="off"
      iconState="active"
      onClick={onClick}
    />
  );
}
