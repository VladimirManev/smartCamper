/**
 * StatusIcons Component
 * Optional per-module WiFi/status indicators (antennas 1–6).
 * Backend connection is not shown — offline backend disables all modules via useModuleStatus.
 */

import { useModuleStatus } from "../hooks/useModuleStatus";
import { isModuleStatusIconsEnabled } from "../utils/moduleGating";
import { SignalIndicator } from "./SignalIndicator";

/**
 * @param {Object} props
 * @param {Object} props.socket - Socket.io instance
 */
export const StatusIcons = ({ socket }) => {
  const { isModuleOnline, getModuleStatus } = useModuleStatus(socket);

  if (!isModuleStatusIconsEnabled()) {
    return null;
  }

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
      label: "Module 4 (Damper & Table Controller)",
    },
    {
      id: "module-5",
      number: "5",
      label: "Module 5 (Appliance Controller)",
    },
    {
      id: "module-6",
      number: "6",
      label: "Module 6 (Victron Energy Monitor)",
    },
  ];

  return (
    <div className="status-icons">
      {moduleIcons.map((module) => {
        const moduleStatus = getModuleStatus(module.id);
        const isOnline = isModuleOnline(module.id);
        const rssi = moduleStatus?.wifiRSSI || null;

        return (
          <SignalIndicator
            key={module.id}
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
