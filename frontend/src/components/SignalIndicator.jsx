/**
 * SignalIndicator Component
 * Displays WiFi signal strength indicator with antenna icon, module number, and signal bars
 * Similar to mobile phone signal indicator
 */

/**
 * Calculate number of bars to show based on RSSI value
 * @param {number|null} rssi - WiFi RSSI value in dBm
 * @returns {number} Number of bars to show (0-4)
 */
const getSignalBars = (rssi) => {
  if (rssi === null || rssi === undefined || rssi === -999) {
    return 0; // No WiFi
  }
  
  if (rssi >= -50) {
    return 4; // Excellent signal (-30 to -50 dBm)
  } else if (rssi >= -60) {
    return 3; // Very good signal (-50 to -60 dBm)
  } else if (rssi >= -70) {
    return 2; // Good signal (-60 to -70 dBm)
  } else if (rssi >= -80) {
    return 1; // Weak signal (-70 to -80 dBm)
  } else {
    return 0; // Very weak signal (below -80 dBm)
  }
};

/**
 * Get color class based on connection status
 * @param {boolean} isOnline - Whether module is online
 * @returns {string} Color class name
 */
const getSignalColor = (isOnline) => {
  return isOnline ? "online" : "offline";
};

/**
 * SignalIndicator component
 * @param {Object} props - Component props
 * @param {string} props.moduleId - Module identifier (e.g., "module-1")
 * @param {string} props.moduleNumber - Module number to display (e.g., "1")
 * @param {boolean} props.isOnline - Whether module is online
 * @param {number|null} props.rssi - WiFi RSSI value in dBm
 * @param {string} props.label - Tooltip label
 */
export const SignalIndicator = ({ moduleId, moduleNumber, isOnline, rssi, label }) => {
  // If module is offline, always show 0 bars regardless of RSSI
  const bars = isOnline ? getSignalBars(rssi) : 0;
  const colorClass = getSignalColor(isOnline);
  
  return (
    <div 
      className={`signal-indicator ${colorClass}`}
      title={label || `Module ${moduleNumber} - Signal: ${isOnline && rssi !== null && rssi !== -999 ? rssi + ' dBm' : 'No signal'}`}
    >
      {/* Module number */}
      <span className="signal-module-number">{moduleNumber}</span>
      
      {/* Signal bars - first is smallest, last is tallest */}
      <div className="signal-bars">
        <div className={`signal-bar bar-1 ${bars >= 1 ? 'active' : ''}`}></div>
        <div className={`signal-bar bar-2 ${bars >= 2 ? 'active' : ''}`}></div>
        <div className={`signal-bar bar-3 ${bars >= 3 ? 'active' : ''}`}></div>
        <div className={`signal-bar bar-4 ${bars >= 4 ? 'active' : ''}`}></div>
      </div>
    </div>
  );
};

