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

      {/* Module status icons */}
      {moduleIcons.map((module) => (
        <span
          key={module.id}
          className={`status-item status-number ${
            isModuleOnline(module.id) ? "online" : "offline"
          }`}
          title={module.label}
        >
          {module.number}
        </span>
      ))}
    </div>
  );
};

