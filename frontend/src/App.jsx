/**
 * App Component
 * Main application component
 * Uses custom hooks and components for separation of concerns
 */

import { useRef } from "react";
import { useSocket } from "./hooks/useSocket";
import { useModuleStatus } from "./hooks/useModuleStatus";
import { useSensorData } from "./hooks/useSensorData";
import { useLEDController } from "./hooks/useLEDController";
import { useFloorHeating } from "./hooks/useFloorHeating";
import { useDamperController } from "./hooks/useDamperController";
import { useTableController } from "./hooks/useTableController";
import { StatusIcons } from "./components/StatusIcons";
import { SensorCard } from "./components/SensorCard";
import { GrayWaterTank } from "./components/GrayWaterTank";
import { LEDCard } from "./components/LEDCard";
import { FloorHeatingCard } from "./components/FloorHeatingCard";
import { DamperCard } from "./components/DamperCard";
import { TableCard } from "./components/TableCard";
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

  // Check if module-4 is online (base module - ready for extension)
  const isModule4Online = isModuleOnline("module-4");

  // LED controller
  const { ledStrips, relays, sendLEDCommand } = useLEDController(socket);

  // Floor heating controller
  const { circles, sendFloorHeatingCommand } = useFloorHeating(socket);

  // Damper controller
  const { dampers, sendDamperCommand } = useDamperController(socket);

  // Table controller
  const { tableState, sendTableCommand } = useTableController(socket);

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
      console.warn("⚠️ Cannot toggle circle - module-3 is offline");
      return;
    }
    
    // Get current mode from state
    const currentMode = circles[index]?.mode || "OFF";
    console.log(`🔥 Toggling circle ${index}, current mode: ${currentMode}`);
    
    // Determine action based on current mode
    // OFF -> on (enable TEMP_CONTROL)
    // TEMP_CONTROL -> off (disable)
    const action = currentMode === "OFF" ? "on" : "off";
    console.log(`🔥 Sending action: ${action}`);
    
    // Send command - state will update when module publishes status
    sendFloorHeatingCommand({
      type: "circle",
      index: index,
      action: action,
    });
  };

  // Damper command handlers with debouncing
  const lastCommandTime = useRef({});
  const DEBOUNCE_DELAY = 300; // ms - prevent multiple rapid clicks
  
  const handleDamperToggle = (index) => {
    // Don't send command if module is offline
    if (!isModule4Online) {
      console.warn("⚠️ Cannot toggle damper - module-4 is offline");
      return;
    }
    
    // Debounce: prevent multiple rapid clicks
    const now = Date.now();
    const lastTime = lastCommandTime.current[index] || 0;
    if (now - lastTime < DEBOUNCE_DELAY) {
      console.log(`⏱️ Debouncing damper ${index} command (${now - lastTime}ms since last)`);
      return;
    }
    lastCommandTime.current[index] = now;
    
    // Get current angle
    const currentAngle = dampers[index]?.angle ?? 90; // Default to 90° if undefined
    
    // Cycle through positions: 0° → 45° → 90° → 0°
    let nextAngle;
    if (currentAngle === 0) {
      nextAngle = 45;
    } else if (currentAngle === 45) {
      nextAngle = 90;
    } else {
      nextAngle = 0;
    }
    
    // Don't send command if already at target angle (shouldn't happen, but safety check)
    if (currentAngle === nextAngle) {
      console.log(`⏭️ Damper ${index} already at ${nextAngle}°, skipping command`);
      return;
    }
    
    console.log(`🌬️ Toggling damper ${index}: ${currentAngle}° → ${nextAngle}°`);
    
    // Send command
    sendDamperCommand({
      type: "damper",
      index: index,
      action: "set_angle",
      angle: nextAngle,
    });
  };

  // Table command handlers
  const handleTableMoveUp = () => {
    if (!isModule4Online) {
      console.warn("⚠️ Cannot move table up - module-4 is offline");
      return;
    }
    console.log("⬆️ Table: Moving up");
    sendTableCommand({
      type: "table",
      action: "move_up",
    });
  };

  const handleTableMoveDown = () => {
    if (!isModule4Online) {
      console.warn("⚠️ Cannot move table down - module-4 is offline");
      return;
    }
    console.log("⬇️ Table: Moving down");
    sendTableCommand({
      type: "table",
      action: "move_down",
    });
  };

  const handleTableStop = () => {
    if (!isModule4Online) {
      return;
    }
    console.log("⏹️ Table: Stopped");
    sendTableCommand({
      type: "table",
      action: "stop",
    });
  };

  const handleTableDoubleClickUp = () => {
    if (!isModule4Online) {
      console.warn("⚠️ Cannot auto move table up - module-4 is offline");
      return;
    }
    console.log("⬆️ Table: Auto moving up for 5 seconds");
    sendTableCommand({
      type: "table",
      action: "move_up_auto",
      duration: 5000,
    });
  };

  const handleTableDoubleClickDown = () => {
    if (!isModule4Online) {
      console.warn("⚠️ Cannot auto move table down - module-4 is offline");
      return;
    }
    console.log("⬇️ Table: Auto moving down for 5 seconds");
    sendTableCommand({
      type: "table",
      action: "move_down_auto",
      duration: 5000,
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

        {/* Dampers */}
        <div className="card-wrapper">
          <DamperCard
            name="Front"
            damper={dampers[0]}
            onClick={() => handleDamperToggle(0)}
            disabled={!isModule4Online}
          />
          <p className="card-label">Front</p>
        </div>

        <div className="card-wrapper">
          <DamperCard
            name="Rear"
            damper={dampers[1]}
            onClick={() => handleDamperToggle(1)}
            disabled={!isModule4Online}
          />
          <p className="card-label">Rear</p>
        </div>

        <div className="card-wrapper">
          <DamperCard
            name="Bath"
            damper={dampers[2]}
            onClick={() => handleDamperToggle(2)}
            disabled={!isModule4Online}
          />
          <p className="card-label">Bath</p>
        </div>

        <div className="card-wrapper">
          <DamperCard
            name="Shoes"
            damper={dampers[3]}
            onClick={() => handleDamperToggle(3)}
            disabled={!isModule4Online}
          />
          <p className="card-label">Shoes</p>
        </div>

        <div className="card-wrapper">
          <DamperCard
            name="Cockpit"
            damper={dampers[4]}
            onClick={() => handleDamperToggle(4)}
            disabled={!isModule4Online}
          />
          <p className="card-label">Cockpit</p>
        </div>

        {/* Table Controls */}
        <div className="card-wrapper">
          <TableCard
            name="Up"
            direction="up"
            tableState={tableState}
            onMouseDown={handleTableMoveUp}
            onMouseUp={handleTableStop}
            onDoubleClick={handleTableDoubleClickUp}
            disabled={!isModule4Online}
          />
          <p className="card-label">Table Up</p>
        </div>

        <div className="card-wrapper">
          <TableCard
            name="Down"
            direction="down"
            tableState={tableState}
            onMouseDown={handleTableMoveDown}
            onMouseUp={handleTableStop}
            onDoubleClick={handleTableDoubleClickDown}
            disabled={!isModule4Online}
          />
          <p className="card-label">Table Down</p>
        </div>
      </div>
    </div>
  );
}

export default App;
