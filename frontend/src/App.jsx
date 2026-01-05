/**
 * App Component
 * Main application component
 * Uses custom hooks and components for separation of concerns
 */

import { useSocket } from "./hooks/useSocket";
import { useModuleStatus } from "./hooks/useModuleStatus";
import { useSensorData } from "./hooks/useSensorData";
import { useLEDController } from "./hooks/useLEDController";
import { useFloorHeating } from "./hooks/useFloorHeating";
import { StatusIcons } from "./components/StatusIcons";
import { SensorCard } from "./components/SensorCard";
import { GrayWaterTank } from "./components/GrayWaterTank";
import { LEDCard } from "./components/LEDCard";
import { FloorHeatingCard } from "./components/FloorHeatingCard";
import "./App.css";

function App() {
  // Socket connection
  const { socket, connected } = useSocket();

  // Module status tracking (from heartbeat system)
  const { isModuleOnline, moduleStatuses } = useModuleStatus(socket);

  // Sensor data (clears when module goes offline)
  const { indoorTemperature, indoorHumidity, outdoorTemperature, grayWaterLevel, grayWaterTemperature } =
    useSensorData(socket, isModuleOnline, moduleStatuses);

  // Check if module-1 is online (provides temperature and humidity)
  const isModule1Online = isModuleOnline("module-1");

  // Check if module-2 is online (provides LED controls)
  const isModule2Online = isModuleOnline("module-2");

  // Check if module-3 is online (provides floor heating controls)
  const isModule3Online = isModuleOnline("module-3");

  // LED controller
  const { ledStrips, relays, sendLEDCommand } = useLEDController(socket);

  // Floor heating controller
  const { circles, sendFloorHeatingCommand } = useFloorHeating(socket);

  // LED command handlers
  const handleStripToggle = (index) => {
    // Don't send command if module is offline
    if (!isModule2Online) {
      return;
    }
    
    sendLEDCommand({
      type: "strip",
      index: index,
      action: ledStrips[index]?.state === "ON" ? "off" : "on",
    });
  };

  const handleBathroomModeCycle = () => {
    // Don't send command if module is offline
    if (!isModule2Online) {
      return;
    }
    
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
    // Don't send command if module is offline
    if (!isModule2Online) {
      return;
    }
    
    sendLEDCommand({
      type: "relay",
      action: "toggle",
    });
  };

  // Floor heating command handlers
  const handleCircleToggle = (index) => {
    // Don't send command if module is offline
    if (!isModule3Online) {
      return;
    }
    
    // Toggle between OFF and TEMP_CONTROL
    const currentMode = circles[index]?.mode || "OFF";
    const newMode = currentMode === "OFF" ? "on" : "off";
    
    sendFloorHeatingCommand({
      type: "circle",
      index: index,
      action: newMode,
    });
  };

  return (
    <div className="app">
      <StatusIcons socket={socket} backendConnected={connected} />

      <div className="main-content">
        {/* Indoor Temperature Sensor */}
        <div className="card-wrapper">
          <SensorCard
            icon="fas fa-thermometer-half"
            value={indoorTemperature}
            unit="°"
            decimals={1}
            disabled={!isModule1Online}
          />
          <p className="card-label">Indoor Temp</p>
        </div>

        {/* Outdoor Temperature Sensor */}
        <div className="card-wrapper">
          <SensorCard
            icon="fas fa-thermometer-half"
            value={outdoorTemperature}
            unit="°"
            decimals={1}
            disabled={!isModule1Online}
          />
          <p className="card-label">Outdoor Temp</p>
        </div>

        {/* Indoor Humidity Sensor */}
        <div className="card-wrapper">
          <SensorCard
            icon="fas fa-tint"
            value={indoorHumidity}
            unit="%"
            decimals={0}
            disabled={!isModule1Online}
          />
          <p className="card-label">Humidity</p>
        </div>

        {/* Gray Water Tank */}
        <div className="card-wrapper">
          <GrayWaterTank
            level={grayWaterLevel}
            temperature={grayWaterTemperature}
            disabled={!isModule1Online}
          />
          <p className="card-label">Gray Water</p>
        </div>

        {/* LED Strips */}
        <div className="card-wrapper">
          <LEDCard
            name="Main"
            strip={ledStrips[1]}
            onClick={() => handleStripToggle(1)}
            type="strip"
            disabled={!isModule2Online}
          />
          <p className="card-label">Main</p>
        </div>

        <div className="card-wrapper">
          <LEDCard
            name="Kitchen"
            strip={ledStrips[0]}
            onClick={() => handleStripToggle(0)}
            type="strip"
            disabled={!isModule2Online}
          />
          <p className="card-label">Kitchen</p>
        </div>

        <div className="card-wrapper">
          <LEDCard
            name="Bedroom"
            strip={ledStrips[4]}
            onClick={() => handleStripToggle(4)}
            type="strip"
            disabled={!isModule2Online}
          />
          <p className="card-label">Bedroom</p>
        </div>

        <div className="card-wrapper">
          <LEDCard
            name="Bathroom"
            strip={ledStrips[3]}
            onClick={handleBathroomModeCycle}
            type="strip"
            disabled={!isModule2Online}
          />
          <p className="card-label">Bathroom</p>
        </div>

        {/* Relay */}
        <div className="card-wrapper">
          <LEDCard
            name="Ambient"
            strip={relays[0]}
            onClick={handleRelayToggle}
            type="relay"
            disabled={!isModule2Online}
          />
          <p className="card-label">Ambient</p>
        </div>

        {/* Floor Heating Circles */}
        <div className="card-wrapper">
          <FloorHeatingCard
            name="Central 1"
            circle={circles[0]}
            onClick={() => handleCircleToggle(0)}
            disabled={!isModule3Online}
          />
          <p className="card-label">Central 1</p>
        </div>

        <div className="card-wrapper">
          <FloorHeatingCard
            name="Central 2"
            circle={circles[1]}
            onClick={() => handleCircleToggle(1)}
            disabled={!isModule3Online}
          />
          <p className="card-label">Central 2</p>
        </div>

        <div className="card-wrapper">
          <FloorHeatingCard
            name="Bathroom"
            circle={circles[2]}
            onClick={() => handleCircleToggle(2)}
            disabled={!isModule3Online}
          />
          <p className="card-label">Bathroom</p>
        </div>

        <div className="card-wrapper">
          <FloorHeatingCard
            name="Podium"
            circle={circles[3]}
            onClick={() => handleCircleToggle(3)}
            disabled={!isModule3Online}
          />
          <p className="card-label">Podium</p>
        </div>
      </div>
    </div>
  );
}

export default App;
