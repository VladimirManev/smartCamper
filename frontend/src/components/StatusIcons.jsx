/**
 * StatusIcons Component
 * Displays connection status icons for backend and modules
 * Uses signal indicators for modules (antenna + number + signal bars)
 */

import { useModuleStatus } from "../hooks/useModuleStatus";
import { SignalIndicator } from "./SignalIndicator";

/**
 * StatusIcons component
 * @param {Object} props - Component props
 * @param {Object} props.socket - Socket.io instance
 * @param {boolean} props.backendConnected - Backend connection status
 */
export const StatusIcons = ({ socket, backendConnected }) => {
  const { isModuleOnline, getModuleStatus } = useModuleStatus(socket);

  // Module icons configuration
  const moduleIcons = [
    {
      id: "module-1",
      number: "1",
      label: "Module 1 (Temperature/Humidity/Water Level)",
    },
    {
      id: "module-2",
      number: "2",
      label: "Module 2 (LED Controller)",
    },
    {
      id: "module-3",
      number: "3",
      label: "Module 3 (Floor Heating Controller)",
    },
    {
      id: "module-4",
      number: "4",
      label: "Module 4 (Base Module - Ready for Extension)",
    },
  ];

  return (
    <div className="status-icons">
      {/* Backend connection status */}
      <span
        className={`status-item status-number ${
          backendConnected ? "online" : "offline"
        }`}
        title="Backend Connection"
      >
        <i className="fas fa-circle"></i>
      </span>

      {/* Module signal indicators */}
      {moduleIcons.map((module) => {
        const moduleStatus = getModuleStatus(module.id);
        const isOnline = isModuleOnline(module.id);
        const rssi = moduleStatus?.wifiRSSI || null;
        
        return (
          <SignalIndicator
            key={module.id}
            moduleId={module.id}
            moduleNumber={module.number}
            isOnline={isOnline}
            rssi={rssi}
            label={module.label}
          />
        );
      })}
    </div>
  );
};

