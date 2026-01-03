/**
 * SensorCard Component
 * Displays sensor reading (temperature, humidity, etc.)
 */

/**
 * SensorCard component
 * @param {Object} props - Component props
 * @param {string} props.icon - FontAwesome icon class
 * @param {string|number|null} props.value - Sensor value
 * @param {string} props.unit - Unit to display (e.g., "°C", "%")
 * @param {number} props.decimals - Number of decimal places (default: 1)
 * @param {boolean} props.disabled - Whether the sensor is disabled/offline
 */
export const SensorCard = ({ icon, value, unit = "", decimals = 1, disabled = false }) => {
  const displayValue =
    value !== null && value !== undefined
      ? typeof value === "number"
        ? value.toFixed(decimals)
        : value
      : "—";

  return (
    <div className={`sensor-card ${disabled ? "disabled" : ""}`}>
      <i className={icon}></i>
      <p className="value">
        {displayValue}
        {displayValue !== "—" && unit}
      </p>
    </div>
  );
};

