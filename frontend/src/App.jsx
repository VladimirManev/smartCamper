/**
 * App Component
 * Main application component
 * Uses custom hooks and components for separation of concerns
 */

import { useRef, useState, useEffect } from "react";
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
import { LEDGroupCard } from "./components/LEDGroupCard";
import { FloorHeatingGroupCard } from "./components/FloorHeatingGroupCard";
import { DamperGroupCard } from "./components/DamperGroupCard";
import { TableGroupCard } from "./components/TableGroupCard";
import { ClockDateCard } from "./components/ClockDateCard";
import { CardModal } from "./components/CardModal";
import { CustomDropdown } from "./components/CustomDropdown";
import ducatoImage from "./assets/ducato.png";
import "./App.css";

function App() {
  // Socket connection
  const { socket, connected } = useSocket();

  // Date state
  const [date, setDate] = useState(new Date());
  
  useEffect(() => {
    const timer = setInterval(() => {
      setDate(new Date());
    }, 1000);
    return () => clearInterval(timer);
  }, []);

  // Format date
  const day = date.getDate();
  const monthNames = [
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  ];
  const month = monthNames[date.getMonth()];
  const dateString = `${day} ${month}`;
  const dayNames = ["SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"];
  const dayName = dayNames[date.getDay()];

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

  // Damper presets - array of { name, angles: [angle0, angle1, angle2, angle3, angle4] }
  // Index: 0=Front, 1=Rear, 2=Bath, 3=Shoes, 4=Cockpit
  const damperPresets = [
    { name: "All Open", angles: [90, 90, 90, 90, 90] },
    { name: "Bath Only", angles: [0, 0, 90, 0, 0] },
    { name: "Cockpit Only", angles: [0, 0, 0, 0, 90] },
    { name: "Rear Only", angles: [0, 90, 0, 0, 0] },
    { name: "Comfort", angles: [45, 90, 45, 45, 0] },
  ];

  // Table controller
  const { tableState, sendTableCommand } = useTableController(socket);

  // Modal state - stack of modals
  const [modalStack, setModalStack] = useState([]);
  
  // Damper preset selection state
  const [selectedPreset, setSelectedPreset] = useState("Manual");
  
  // Track expected angles when applying preset (to detect physical button changes)
  const expectedPresetAngles = useRef(null);
  const isApplyingPreset = useRef(false);

  const openModal = (cardType, cardName, cardData = null) => {
    setModalStack((prevStack) => [
      ...prevStack,
      { cardType, cardName, cardData },
    ]);
  };

  const closeModal = () => {
    setModalStack((prevStack) => prevStack.slice(0, -1));
  };

  // Get current (top) modal
  const currentModal = modalStack.length > 0 ? modalStack[modalStack.length - 1] : null;

  // Render modal content based on cardType
  const renderModalContent = (modal) => {
    if (!modal) return null;

    const { cardType } = modal;

    if (cardType === "lighting-group") {
      // Render all LED cards in grid
      return (
        <div className="modal-grid">
          <div className="card-wrapper">
            <LEDCard
              name="Main"
              strip={ledStrips[1]}
              onClick={() => handleStripToggle(1)}
              onLongPress={() => openModal("led", "Main", { strip: ledStrips[1], type: "strip", index: 1 })}
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
              onLongPress={() => openModal("led", "Kitchen", { strip: ledStrips[0], type: "strip", index: 0 })}
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
              onLongPress={() => openModal("led", "Bedroom", { strip: ledStrips[4], type: "strip", index: 4 })}
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
              onLongPress={() => openModal("led", "Bathroom", { strip: ledStrips[3], type: "strip", index: 3 })}
              type="strip"
              disabled={!isModule2Online}
            />
            <p className="card-label">Bathroom</p>
          </div>
          <div className="card-wrapper">
            <LEDCard
              name="Ambient"
              strip={relays[0]}
              onClick={handleRelayToggle}
              onLongPress={() => openModal("led", "Ambient", { strip: relays[0], type: "relay", index: 0 })}
              type="relay"
              disabled={!isModule2Online}
            />
            <p className="card-label">Ambient</p>
          </div>
        </div>
      );
    }

    if (cardType === "floor-heating-group") {
      // Render all floor heating cards in grid
      return (
        <div className="modal-grid">
          <div className="card-wrapper">
            <FloorHeatingCard
              name="Central 1"
              circle={circles[0]}
              onClick={() => handleCircleToggle(0)}
              onLongPress={() => openModal("floor-heating", "Central 1", { circle: circles[0], index: 0 })}
              disabled={!isModule3Online}
            />
            <p className="card-label">Central 1</p>
          </div>
          <div className="card-wrapper">
            <FloorHeatingCard
              name="Central 2"
              circle={circles[1]}
              onClick={() => handleCircleToggle(1)}
              onLongPress={() => openModal("floor-heating", "Central 2", { circle: circles[1], index: 1 })}
              disabled={!isModule3Online}
            />
            <p className="card-label">Central 2</p>
          </div>
          <div className="card-wrapper">
            <FloorHeatingCard
              name="Bathroom"
              circle={circles[2]}
              onClick={() => handleCircleToggle(2)}
              onLongPress={() => openModal("floor-heating", "Bathroom", { circle: circles[2], index: 2 })}
              disabled={!isModule3Online}
            />
            <p className="card-label">Bathroom</p>
          </div>
          <div className="card-wrapper">
            <FloorHeatingCard
              name="Podium"
              circle={circles[3]}
              onClick={() => handleCircleToggle(3)}
              onLongPress={() => openModal("floor-heating", "Podium", { circle: circles[3], index: 3 })}
              disabled={!isModule3Online}
            />
            <p className="card-label">Podium</p>
          </div>
        </div>
      );
    }

    if (cardType === "damper-group") {
      // Render all damper cards in grid
      return (
        <div>
          <div className="damper-preset-container">
            <CustomDropdown
              value={selectedPreset}
              onChange={(presetName) => {
                setSelectedPreset(presetName);
                const preset = damperPresets.find(p => p.name === presetName);
                if (preset) {
                  handleDamperPreset(preset);
                }
              }}
              options={[
                { value: "Manual", label: "Manual" },
                ...damperPresets.map(preset => ({
                  value: preset.name,
                  label: preset.name
                }))
              ]}
              placeholder="Select preset..."
              disabled={!isModule4Online}
            />
          </div>
          <div className="modal-grid">
          <div className="card-wrapper">
            <DamperCard
              name="Front"
              damper={dampers[0]}
              onClick={() => handleDamperToggle(0)}
              onLongPress={() => openModal("damper", "Front", { damper: dampers[0], index: 0 })}
              disabled={!isModule4Online}
            />
            <p className="card-label">Front</p>
          </div>
          <div className="card-wrapper">
            <DamperCard
              name="Rear"
              damper={dampers[1]}
              onClick={() => handleDamperToggle(1)}
              onLongPress={() => openModal("damper", "Rear", { damper: dampers[1], index: 1 })}
              disabled={!isModule4Online}
            />
            <p className="card-label">Rear</p>
          </div>
          <div className="card-wrapper">
            <DamperCard
              name="Bath"
              damper={dampers[2]}
              onClick={() => handleDamperToggle(2)}
              onLongPress={() => openModal("damper", "Bath", { damper: dampers[2], index: 2 })}
              disabled={!isModule4Online}
            />
            <p className="card-label">Bath</p>
          </div>
          <div className="card-wrapper">
            <DamperCard
              name="Shoes"
              damper={dampers[3]}
              onClick={() => handleDamperToggle(3)}
              onLongPress={() => openModal("damper", "Shoes", { damper: dampers[3], index: 3 })}
              disabled={!isModule4Online}
            />
            <p className="card-label">Shoes</p>
          </div>
          <div className="card-wrapper">
            <DamperCard
              name="Cockpit"
              damper={dampers[4]}
              onClick={() => handleDamperToggle(4)}
              onLongPress={() => openModal("damper", "Cockpit", { damper: dampers[4], index: 4 })}
              disabled={!isModule4Online}
            />
            <p className="card-label">Cockpit</p>
          </div>
          </div>
        </div>
      );
    }

    if (cardType === "table-group") {
      // Render all table cards in column: auto(up), hold(up), hold(down), auto(down)
      return (
        <div className="modal-grid table-modal-column">
          {/* Auto card: Auto Up */}
          <div className="card-wrapper">
            <TableCard
              name="Up"
              direction="up"
              tableState={tableState}
              onClick={() => handleTableClick("up")}
              isAuto={true}
              disabled={!isModule4Online}
            />
          </div>
          {/* Hold card: Hold Up */}
          <div className="card-wrapper">
            <TableCard
              name="Up"
              direction="up"
              tableState={tableState}
              onHoldStart={() => handleTableHoldStart("up")}
              onHoldEnd={handleTableHoldEnd}
              disabled={!isModule4Online}
            />
          </div>
          {/* Hold card: Hold Down */}
          <div className="card-wrapper">
            <TableCard
              name="Down"
              direction="down"
              tableState={tableState}
              onHoldStart={() => handleTableHoldStart("down")}
              onHoldEnd={handleTableHoldEnd}
              disabled={!isModule4Online}
            />
          </div>
          {/* Auto card: Auto Down */}
          <div className="card-wrapper">
            <TableCard
              name="Down"
              direction="down"
              tableState={tableState}
              onClick={() => handleTableClick("down")}
              isAuto={true}
              disabled={!isModule4Online}
            />
          </div>
        </div>
      );
    }

    // For individual card modals (led, floor-heating, damper) - empty for now
    return null;
  };

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
    
    // Cycle through positions: 90° → 45° → 0° → 90°
    let nextAngle;
    if (currentAngle === 90) {
      nextAngle = 45;
    } else if (currentAngle === 45) {
      nextAngle = 0;
    } else {
      nextAngle = 90;
    }
    
    // Don't send command if already at target angle (shouldn't happen, but safety check)
    if (currentAngle === nextAngle) {
      console.log(`⏭️ Damper ${index} already at ${nextAngle}°, skipping command`);
      return;
    }
    
    console.log(`🌬️ Toggling damper ${index}: ${currentAngle}° → ${nextAngle}°`);
    
    // If a preset is selected, switch to Manual when manually changing a damper
    if (selectedPreset !== "Manual") {
      setSelectedPreset("Manual");
      expectedPresetAngles.current = null;
    }
    
    // Send command
    sendDamperCommand({
      type: "damper",
      index: index,
      action: "set_angle",
      angle: nextAngle,
    });
  };

  // Damper preset handler
  const handleDamperPreset = (preset) => {
    if (!isModule4Online) {
      console.warn("⚠️ Cannot apply preset - module-4 is offline");
      return;
    }

    console.log(`🌬️ Applying preset: ${preset.name}`);
    
    // Store expected angles and set flag to ignore updates temporarily
    expectedPresetAngles.current = preset.angles;
    isApplyingPreset.current = true;
    
    // Create array of commands with index and angle, then sort by angle (descending)
    // This ensures we open dampers first, then close them (protection: at least one must be open)
    const commands = preset.angles.map((angle, index) => ({ index, angle }));
    commands.sort((a, b) => b.angle - a.angle); // Sort descending: open first, close last
    
    // Send commands in sorted order with delay between each (to allow module to process)
    const COMMAND_DELAY = 200; // ms delay between commands
    commands.forEach(({ index, angle }, commandIndex) => {
      setTimeout(() => {
        sendDamperCommand({
          type: "damper",
          index: index,
          action: "set_angle",
          angle: angle,
        });
      }, commandIndex * COMMAND_DELAY);
    });
    
    // Clear flag after 2 seconds (enough time for all updates to arrive)
    setTimeout(() => {
      isApplyingPreset.current = false;
    }, 2000);
  };

  // Detect physical button changes - switch to Manual if preset doesn't match
  useEffect(() => {
    // Skip if we're currently applying a preset (waiting for updates)
    if (isApplyingPreset.current) {
      return;
    }
    
    // Skip if no preset is selected (already Manual)
    if (selectedPreset === "Manual" || !expectedPresetAngles.current) {
      return;
    }
    
    // Check if current angles match expected preset angles
    const currentAngles = [0, 1, 2, 3, 4].map(i => dampers[i]?.angle ?? 90);
    const expectedAngles = expectedPresetAngles.current;
    
    // Compare angles - if any doesn't match, switch to Manual
    const anglesMatch = currentAngles.every((angle, index) => angle === expectedAngles[index]);
    
    if (!anglesMatch) {
      console.log("🌬️ Physical button change detected - switching to Manual");
      setSelectedPreset("Manual");
      expectedPresetAngles.current = null;
    }
  }, [dampers, selectedPreset]);

  // Table command handlers - simplified: single click triggers auto movement
  const lastTableCommandTime = useRef(0);
  const lastTableCommandDirection = useRef(null);
  const TABLE_COMMAND_DEBOUNCE = 500; // ms - prevent duplicate commands before status update
  
  const handleTableClick = (direction) => {
    if (!isModule4Online) {
      console.warn(`⚠️ Cannot move table ${direction} - module-4 is offline`);
      return;
    }
    
    const now = Date.now();
    
    // CRITICAL: If auto-moving, stop immediately on any button press
    if (tableState?.autoMoving) {
      console.log("⏹️ Table: Auto movement stopped by button press");
      sendTableCommand({
        type: "table",
        action: "stop",
      });
      lastTableCommandTime.current = now;
      lastTableCommandDirection.current = null;
      return;
    }
    
    // Debounce: prevent duplicate commands in same direction before status update
    // This handles race condition where command is sent but status not yet received
    if (
      lastTableCommandDirection.current === direction &&
      now - lastTableCommandTime.current < TABLE_COMMAND_DEBOUNCE
    ) {
      console.log(`⏱️ Table: Ignoring duplicate ${direction} command (debounce)`);
      return;
    }
    
    // Start auto movement in the specified direction
    const arrow = direction === "up" ? "⬆️" : "⬇️";
    console.log(
      `${arrow} Table: Auto moving ${direction} (duration controlled by ESP32)`
    );
    sendTableCommand({
      type: "table",
      action: direction === "up" ? "move_up_auto" : "move_down_auto",
      // duration премахнато - ESP32 използва своята константа TABLE_AUTO_MOVE_DURATION
    });
    
    // Track last command
    lastTableCommandTime.current = now;
    lastTableCommandDirection.current = direction;
  };

  // Table hold/release handlers for manual control
  const handleTableHoldStart = (direction) => {
    if (!isModule4Online) {
      console.warn(`⚠️ Cannot hold table ${direction} - module-4 is offline`);
      return;
    }
    
    // Stop any auto movement first
    if (tableState?.autoMoving) {
      sendTableCommand({
        type: "table",
        action: "stop",
      });
    }
    
    // Start continuous movement
    const arrow = direction === "up" ? "⬆️" : "⬇️";
    console.log(`${arrow} Table: Holding ${direction} - continuous movement`);
    sendTableCommand({
      type: "table",
      action: direction === "up" ? "move_up" : "move_down",
    });
  };

  const handleTableHoldEnd = () => {
    if (!isModule4Online) {
      return;
    }
    
    // Stop movement when released
    console.log("⏹️ Table: Released - stopping movement");
    sendTableCommand({
      type: "table",
      action: "stop",
    });
  };

  return (
    <div className="app">
      <StatusIcons socket={socket} backendConnected={connected} />

      <div className="content-with-image">
        {/* Date and sensor text labels: Date, IN temp, IN humidity, OUT temp */}
        <div className="sensor-text-row">
          <div className="sensor-text-item date-text-item">
            <div className="sensor-text-content">
              <span className="sensor-label date-label">{dayName}</span>
              <span className="sensor-value date-value">{dateString}</span>
            </div>
          </div>
          <div className="sensor-text-item">
            <i className="fas fa-thermometer-half sensor-icon"></i>
            <div className="sensor-text-content">
              <span className="sensor-label">IN</span>
              <span className="sensor-value">
                {indoorTemperature !== null && indoorTemperature !== undefined 
                  ? `${indoorTemperature.toFixed(1)}°` 
                  : "--°"}
              </span>
            </div>
          </div>
          <div className="sensor-text-item">
            <i className="fas fa-tint sensor-icon"></i>
            <div className="sensor-text-content">
              <span className="sensor-label">IN</span>
              <span className="sensor-value">
                {indoorHumidity !== null && indoorHumidity !== undefined 
                  ? `${indoorHumidity.toFixed(0)}%` 
                  : "--%"}
              </span>
            </div>
          </div>
          <div className="sensor-text-item">
            <i className="fas fa-thermometer-half sensor-icon"></i>
            <div className="sensor-text-content">
              <span className="sensor-label">OUT</span>
              <span className="sensor-value">
                {outdoorTemperature !== null && outdoorTemperature !== undefined 
                  ? `${outdoorTemperature.toFixed(1)}°` 
                  : "--°"}
              </span>
            </div>
          </div>
        </div>
        
        <div className="image-clock-container">
          <ClockDateCard 
            indoorTemp={indoorTemperature}
            outdoorTemp={outdoorTemperature}
            humidity={indoorHumidity}
          />
          <div className="image-sensor-wrapper">
            <div className="image-container">
              <img src={ducatoImage} alt="Ducato" className="ducato-image" />
            </div>
          </div>
        </div>

        <div className="main-content">
        {/* LED Group Card */}
        <div className="card-wrapper">
          <LEDGroupCard
            name="Lighting"
            onClick={() => openModal("lighting-group", "Lighting")}
            disabled={!isModule2Online}
          />
          <p className="card-label">Lighting</p>
        </div>

        {/* Floor Heating Group Card */}
        <div className="card-wrapper">
          <FloorHeatingGroupCard
            name="Floor Heating"
            onClick={() => openModal("floor-heating-group", "Floor Heating")}
            disabled={!isModule3Online}
          />
          <p className="card-label">Floor Heating</p>
        </div>

        {/* Dampers Group Card */}
        <div className="card-wrapper">
          <DamperGroupCard
            name="Airflow"
            onClick={() => openModal("damper-group", "Airflow")}
            disabled={!isModule4Online}
          />
          <p className="card-label">Airflow</p>
        </div>

        {/* Table Group Card */}
        <div className="card-wrapper">
          <TableGroupCard
            name="Table"
            onClick={() => openModal("table-group", "Table")}
            disabled={!isModule4Online}
          />
          <p className="card-label">Table</p>
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
      </div>
      </div>

      {/* Card Modal - render all modals in stack */}
      {modalStack.map((modal, index) => {
        const isTopModal = index === modalStack.length - 1;
        return (
          <CardModal
            key={index}
            isOpen={true}
            onClose={isTopModal ? closeModal : null}
            title={modal.cardName}
            isNested={!isTopModal}
            zIndex={1000 + index}
          >
            {renderModalContent(modal)}
          </CardModal>
        );
      })}
    </div>
  );
}

export default App;
