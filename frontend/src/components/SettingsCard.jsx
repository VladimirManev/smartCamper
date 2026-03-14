/**
 * SettingsCard Component
 * Settings card with gear icon
 * Opens a modal when clicked, logs to console on long press
 */

import { Card } from "./Card";

/**
 * SettingsCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Settings")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {Function} props.onLongPress - Long press handler function (logs to console)
 * @param {boolean} props.disabled - Whether the control is disabled
 */
export const SettingsCard = ({ name, onClick, onLongPress, disabled = false }) => {
  // Settings icon (FontAwesome)
  const settingsIcon = <i className="fas fa-cog"></i>;

  return (
    <Card
      name={name}
      icon={settingsIcon}
      buttonState="off"
      iconState="active"
      onClick={onClick}
      onLongPress={onLongPress}
      disabled={disabled}
    />
  );
};
