/**
 * App Component
 * Main application component
 * Uses custom hooks and components for separation of concerns
 */

import { useSocket } from "./hooks/useSocket";
import { useModuleStatus } from "./hooks/useModuleStatus";
import { useSensorData } from "./hooks/useSensorData";
import { useLEDController } from "./hooks/useLEDController";
import { StatusIcons } from "./components/StatusIcons";
import { SensorCard } from "./components/SensorCard";
import { GrayWaterTank } from "./components/GrayWaterTank";
import { LEDCard } from "./components/LEDCard";
import "./App.css";

function App() {
  // Socket connection
  const { socket, connected } = useSocket();

  // Module status tracking (from heartbeat system)
  const { isModuleOnline, moduleStatuses } = useModuleStatus(socket);

  // Sensor data (clears when module goes offline)
  const { temperature, humidity, grayWaterLevel } = useSensorData(
    socket,
    isModuleOnline,
    moduleStatuses
  );
  
  // Check if module-1 is online (provides temperature and humidity)
  const isModule1Online = isModuleOnline("module-1");

  // LED controller
  const { ledStrips, relays, sendLEDCommand } = useLEDController(socket);

  // LED command handlers
  const handleStripToggle = (index) => {
    sendLEDCommand({
      type: "strip",
      index: index,
      action: ledStrips[index]?.state === "ON" ? "off" : "on",
    });
  };

  const handleBathroomModeCycle = () => {
    const currentMode = ledStrips[3]?.mode || "OFF";
    let nextMode;
    // Cycle: OFF -> AUTO -> ON -> OFF
    if (currentMode === "OFF") {
      nextMode = "AUTO";
    } else if (currentMode === "AUTO") {
      nextMode = "ON";
    } else {
      nextMode = "OFF";
    }

    sendLEDCommand({
      type: "strip",
      index: 3,
      action: "mode",
      value: nextMode,
    });
  };

  const handleRelayToggle = () => {
    sendLEDCommand({
      type: "relay",
      action: "toggle",
    });
  };

  return (
    <div className="app">
      <StatusIcons socket={socket} backendConnected={connected} />

      <div className="main-content">
        {/* Temperature Sensor */}
        <SensorCard
          icon="fas fa-thermometer-half"
          value={temperature}
          unit="°C"
          decimals={1}
          disabled={!isModule1Online}
        />

        {/* Humidity Sensor */}
        <SensorCard
          icon="fas fa-tint"
          value={humidity}
          unit="%"
          decimals={0}
          disabled={!isModule1Online}
        />

        {/* Gray Water Tank */}
        <GrayWaterTank level={grayWaterLevel} disabled={!isModule1Online} />

        {/* LED Strips */}
        <LEDCard
          name="Kitchen"
          strip={ledStrips[0]}
          onClick={() => handleStripToggle(0)}
          type="strip"
        />

        <LEDCard
          name="Lighting"
          strip={ledStrips[1]}
          onClick={() => handleStripToggle(1)}
          type="strip"
        />

        <LEDCard
          name="Bedroom"
          strip={ledStrips[4]}
          onClick={() => handleStripToggle(4)}
          type="strip"
        />

        <LEDCard
          name="Bathroom"
          strip={ledStrips[3]}
          onClick={handleBathroomModeCycle}
          type="strip"
        />

        {/* Relay */}
        <LEDCard
          name="Floor"
          strip={relays[0]}
          onClick={handleRelayToggle}
          type="relay"
        />
      </div>
    </div>
  );
}

export default App;
