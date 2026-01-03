/**
 * StatusIcons Component
 * Displays connection status icons for backend and modules
 */

import { useModuleStatus } from "../hooks/useModuleStatus";

/**
 * StatusIcons component
 * @param {Object} props - Component props
 * @param {Object} props.socket - Socket.io instance
 * @param {boolean} props.backendConnected - Backend connection status
 */
export const StatusIcons = ({ socket, backendConnected }) => {
  const { isModuleOnline } = useModuleStatus(socket);

  // Module icons configuration
  const moduleIcons = [
    {
      id: "module-1",
      icon: "fa-thermometer-half",
      label: "Module 1 (Temperature/Humidity)",
    },
    {
      id: "led-controller",
      icon: "fa-lightbulb",
      label: "LED Controller",
    },
    {
      id: "gray-water-sensor",
      icon: "fa-water",
      label: "Gray Water Sensor",
    },
  ];

  return (
    <div className="status-icons">
      {/* Backend connection status */}
      <span className="status-item" title="Backend Connection">
        <i className={`fas fa-circle ${backendConnected ? "online" : "offline"}`}></i>
      </span>

      {/* Module status icons */}
      {moduleIcons.map((module) => (
        <span
          key={module.id}
          className="status-item"
          title={module.label}
        >
          <i
            className={`fas ${module.icon} ${
              isModuleOnline(module.id) ? "online" : "offline"
            }`}
          ></i>
        </span>
      ))}
    </div>
  );
};

