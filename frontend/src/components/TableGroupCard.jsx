/**
 * TableGroupCard Component
 * Group card that displays as a table card in OFF state
 * Opens a modal with all table cards when clicked
 */

import { Card } from "./Card";

/**
 * TableGroupCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Table")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const TableGroupCard = ({ name, onClick, disabled = false }) => {
  // Table icon SVG (from table.svg - converted to use currentColor)
  const tableIcon = (
    <svg viewBox="0 0 32 32" fill="none" xmlns="http://www.w3.org/2000/svg">
      <polyline points="25,25 21,29 17,25" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round" strokeMiterlimit="10"/>
      <line x1="21" y1="29" x2="21" y2="7" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round" strokeMiterlimit="10"/>
      <polyline points="15,7 11,3 7,7" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round" strokeMiterlimit="10"/>
      <line x1="11" y1="3" x2="11" y2="25" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round" strokeMiterlimit="10"/>
    </svg>
  );

  return (
    <Card
      name={name}
      icon={tableIcon}
      buttonState="off"
      iconState="active"
      onClick={onClick}
      disabled={disabled}
    />
  );
};

