/**
 * DamperGroupCard Component
 * Group card that displays as a damper card in OFF/closed state
 * Opens a modal with all damper cards when clicked
 */

import { Card } from "./Card";

/**
 * DamperGroupCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Dampers")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const DamperGroupCard = ({ name, onClick, disabled = false }) => {
  // Airflow icon SVG (from airflow.svg - converted to use currentColor)
  const airflowIcon = (
    <svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path d="M8 12H14M16.5 4C17.8807 4 19 5.11929 19 6.5C19 7.88071 17.8807 9 16.5 9H14M5 9H10M17 19C18.1046 19 19 18.1046 19 17C19 15.8954 18.1046 15 17 15H11M4 15H7" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"/>
    </svg>
  );

  return (
    <Card
      name={name}
      icon={airflowIcon}
      buttonState="off"
      iconState="active"
      onClick={onClick}
      disabled={disabled}
    />
  );
};

