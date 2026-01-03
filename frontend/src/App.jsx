import { useState, useEffect, useRef } from "react";
import io from "socket.io-client";
import "./App.css";

function App() {
  // State for temperature and humidity
  const [temperature, setTemperature] = useState(null);
  const [humidity, setHumidity] = useState(null);
  const [connected, setConnected] = useState(false);
  const [esp32Connected, setEsp32Connected] = useState(false);

  // State for gray water level
  const [grayWaterLevel, setGrayWaterLevel] = useState(null);
  const [grayWaterConnected, setGrayWaterConnected] = useState(false);

  // State for LED controller
  const [ledControllerConnected, setLedControllerConnected] = useState(false);
  const [ledStrips, setLedStrips] = useState({
    0: { state: "OFF", brightness: 0 }, // Kitchen
    1: { state: "OFF", brightness: 0 }, // Lighting
    3: { state: "OFF", brightness: 0, mode: "OFF" }, // Bathroom (motion-activated)
  });
  const [relays, setRelays] = useState({
    0: { state: "OFF" }, // Relay 0
  });

  // Store socket reference for use in buttons
  const socketRef = useRef(null);

  // Helper function to calculate arc progress
  // Arc is from 135° to 45° = 270 degrees total
  // Fills according to brightness value
  const getArcProgress = (brightness, isOn) => {
    if (!isOn || brightness === 0) {
      return 0;
    }
    // Arc length: 270 degrees = π * radius * (270/180) ≈ 377px (for radius 80)
    const arcLength = Math.PI * 80 * (270 / 180);

    // Progress from 0 to 1 according to brightness (0-255)
    const progress = brightness / 255;

    return progress * arcLength;
  };

  // useEffect = executes when component loads
  useEffect(() => {
    // Connect to backend WebSocket
    // In development: use Raspberry Pi IP
    // In production: use same host (relative URL)
    const isDevelopment = import.meta.env.DEV;
    const socketUrl = isDevelopment
      ? "http://192.168.4.1:3000" // Raspberry Pi IP
      : window.location.origin; // Production - same host
    const socket = io(socketUrl);
    socketRef.current = socket; // Store reference

    // When connected
    socket.on("connect", () => {
      console.log("✅ Connected to backend");
      setConnected(true);
      // ESP32 status remains false until we receive data
      setEsp32Connected(false); // Force reset ESP32 status
      setGrayWaterConnected(false); // Force reset gray water status
    });

    // When disconnected
    socket.on("disconnect", () => {
      console.log("❌ Disconnected from backend");
      setConnected(false);
      setEsp32Connected(false); // Reset ESP32 status on backend disconnect
      setGrayWaterConnected(false); // Reset gray water status on backend disconnect
    });

    // Timeout for ESP32 - global variable
    let esp32Timeout;
    let ledControllerTimeout;
    let grayWaterTimeout;

    // Listen for sensor updates
    socket.on("sensorUpdate", (data) => {
      console.log("📊 New data:", data);
      setTemperature(data.temperature);
      setHumidity(data.humidity);

      // Handle gray water level
      if (data.grayWaterLevel !== undefined && data.grayWaterLevel !== null) {
        setGrayWaterLevel(data.grayWaterLevel);
        setGrayWaterConnected(true);

        // Restart timeout for gray water sensor
        clearTimeout(grayWaterTimeout);
        grayWaterTimeout = setTimeout(() => {
          setGrayWaterConnected(false);
          setGrayWaterLevel(null);
        }, 30000); // 30 second timeout
      }

      // Handle temperature/humidity (ESP32 sensor)
      if (data.temperature !== undefined || data.humidity !== undefined) {
        setEsp32Connected(true);
        // Restart timeout every time we receive data
        clearTimeout(esp32Timeout);
        esp32Timeout = setTimeout(() => {
          setEsp32Connected(false);
          setTemperature(null); // Clear temperature
          setHumidity(null); // Clear humidity
        }, 30000); // 30 second timeout (20 second reserve after ESP32 heartbeat)
      }
    });

    // Heartbeat is now handled via ledStatusUpdate (full status at intervals)

    // Listen for LED status updates
    socket.on("ledStatusUpdate", (data) => {
      console.log("💡 LED Status Update:", data);

      // Every status update means module is alive - update heartbeat
      setLedControllerConnected(true);
      clearTimeout(ledControllerTimeout);
      ledControllerTimeout = setTimeout(() => {
        setLedControllerConnected(false);
      }, 30000); // 30 second timeout

      // NEW FORMAT: Full status in one object
      if (data.type === "full" && data.data) {
        const statusData = data.data;

        // Update all strips
        if (statusData.strips) {
          const newStrips = {};
          for (const [index, stripData] of Object.entries(statusData.strips)) {
            newStrips[index] = {
              state: stripData.state,
              brightness: stripData.brightness,
              ...(stripData.mode && { mode: stripData.mode }), // Add mode if present (for Strip 3)
            };
          }
          setLedStrips(newStrips);
        }

        // Update all relays (format like strips)
        if (statusData.relays) {
          const newRelays = {};
          for (const [index, relayData] of Object.entries(statusData.relays)) {
            newRelays[index] = {
              state: relayData.state,
            };
          }
          setRelays(newRelays);
        }
      }
      // OLD FORMAT (for backward compatibility)
      else if (data.type === "strip" && typeof data.index === "number") {
        setLedStrips((prev) => ({
          ...prev,
          [data.index]: {
            ...prev[data.index],
            [data.dataType]: data.value,
          },
        }));
      } else if (data.type === "relay") {
        // Old format - update relay 0
        setRelays((prev) => ({
          ...prev,
          0: { state: data.value },
        }));
      }
    });

    // Do NOT start timeout immediately - icons should be red until data is received

    // Cleanup function - disconnect socket when component unmounts
    return () => {
      socket.disconnect();
      clearTimeout(esp32Timeout);
      clearTimeout(ledControllerTimeout);
      clearTimeout(grayWaterTimeout);
    };
  }, []); // [] = execute only once on load

  return (
    <div className="app">
      <div className="status-icons">
        <span className="status-item">
          <i
            className={`fas fa-circle ${connected ? "online" : "offline"}`}
          ></i>
        </span>
        <span className="status-item">
          <i
            className={`fas fa-thermometer-half ${
              esp32Connected ? "online" : "offline"
            }`}
          ></i>
        </span>
        <span className="status-item">
          <i
            className={`fas fa-lightbulb ${
              ledControllerConnected ? "online" : "offline"
            }`}
          ></i>
        </span>
        <span className="status-item">
          <i
            className={`fas fa-water ${
              grayWaterConnected ? "online" : "offline"
            }`}
          ></i>
        </span>
      </div>

      <div className="main-content">
        <div className="sensor-card">
          <i className="fas fa-thermometer-half"></i>
          <p className="value">
            {temperature !== null ? `${temperature.toFixed(1)}°C` : "—"}
          </p>
        </div>

        <div className="sensor-card">
          <i className="fas fa-tint"></i>
          <p className="value">{humidity !== null ? `${humidity}%` : "—"}</p>
        </div>

        <div className="sensor-card water-tank-card">
          <p className="water-tank-label">Gray Water</p>
          <div className="water-tank-container">
            <svg
              className="water-tank"
              viewBox="0 0 100 100"
              preserveAspectRatio="xMidYMid meet"
            >
              {/* Tank outline */}
              <rect
                x="25"
                y="10"
                width="50"
                height="80"
                fill="none"
                stroke="#b3e5b3"
                strokeWidth="2"
                rx="3"
              />

              {/* Water fill - fills from bottom up */}
              {grayWaterLevel !== null && (
                <rect
                  x="27"
                  y={90 - (grayWaterLevel / 100) * 80}
                  width="46"
                  height={(grayWaterLevel / 100) * 80}
                  fill="url(#grayWaterGradient)"
                  rx="2"
                  style={{
                    transition: "y 0.5s ease, height 0.5s ease",
                  }}
                />
              )}

              {/* Gray water gradient */}
              <defs>
                <linearGradient
                  id="grayWaterGradient"
                  x1="0%"
                  y1="0%"
                  x2="0%"
                  y2="100%"
                >
                  <stop offset="0%" stopColor="#95a5a6" stopOpacity="0.8" />
                  <stop offset="50%" stopColor="#7f8c8d" stopOpacity="0.9" />
                  <stop offset="100%" stopColor="#5d6d7e" stopOpacity="1" />
                </linearGradient>
              </defs>

              {/* Water level indicator lines (optional) */}
              <line
                x1="22"
                y1="30"
                x2="25"
                y2="30"
                stroke="#b3e5b3"
                strokeWidth="1"
                opacity="0.5"
              />
              <line
                x1="22"
                y1="50"
                x2="25"
                y2="50"
                stroke="#b3e5b3"
                strokeWidth="1"
                opacity="0.5"
              />
              <line
                x1="22"
                y1="70"
                x2="25"
                y2="70"
                stroke="#b3e5b3"
                strokeWidth="1"
                opacity="0.5"
              />
            </svg>
          </div>
        </div>

        <div
          className="led-card"
          onClick={() => {
            if (socketRef.current) {
              socketRef.current.emit("ledCommand", {
                type: "strip",
                index: 0,
                action: ledStrips[0]?.state === "ON" ? "off" : "on",
              });
            }
          }}
        >
          <p className="led-name">Kitchen</p>
          <div
            className={`neumorphic-button ${
              ledStrips[0]?.state === "ON" ? "on" : "off"
            }`}
          >
            {(() => {
              const arcLength = Math.PI * 80 * (270 / 180);
              const progress = getArcProgress(
                ledStrips[0]?.brightness || 0,
                ledStrips[0]?.state === "ON"
              );
              return (
                <svg className="horseshoe-progress" viewBox="0 0 200 200">
                  <defs>
                    <linearGradient
                      id="gradient-0"
                      x1="0%"
                      y1="0%"
                      x2="0%"
                      y2="100%"
                    >
                      <stop offset="0%" stopColor="#00C6FF" />
                      <stop offset="100%" stopColor="#00FF99" />
                    </linearGradient>
                  </defs>
                  {/* Arc from 135° (start) to 45° (end) - fills according to brightness */}
                  <path
                    className="horseshoe-fill"
                    d="M 43.4 156.6 A 80 80 0 1 1 156.6 156.6"
                    fill="none"
                    stroke="url(#gradient-0)"
                    strokeWidth="8"
                    strokeLinecap="round"
                    strokeDasharray={`${progress} ${arcLength}`}
                    strokeDashoffset="0"
                    opacity={
                      ledStrips[0]?.state === "ON" && progress > 0 ? 1 : 0
                    }
                  />
                </svg>
              );
            })()}
            <span className="button-text">{ledStrips[0]?.state || "OFF"}</span>
          </div>
        </div>

        <div
          className="led-card"
          onClick={() => {
            if (socketRef.current) {
              socketRef.current.emit("ledCommand", {
                type: "strip",
                index: 1,
                action: ledStrips[1]?.state === "ON" ? "off" : "on",
              });
            }
          }}
        >
          <p className="led-name">Lighting</p>
          <div
            className={`neumorphic-button ${
              ledStrips[1]?.state === "ON" ? "on" : "off"
            }`}
          >
            {(() => {
              const arcLength = Math.PI * 80 * (270 / 180);
              const progress = getArcProgress(
                ledStrips[1]?.brightness || 0,
                ledStrips[1]?.state === "ON"
              );
              return (
                <svg className="horseshoe-progress" viewBox="0 0 200 200">
                  <defs>
                    <linearGradient
                      id="gradient-1"
                      x1="0%"
                      y1="0%"
                      x2="0%"
                      y2="100%"
                    >
                      <stop offset="0%" stopColor="#00C6FF" />
                      <stop offset="100%" stopColor="#00FF99" />
                    </linearGradient>
                  </defs>
                  {/* Arc from 135° (start) to 45° (end) - fills according to brightness */}
                  <path
                    className="horseshoe-fill"
                    d="M 43.4 156.6 A 80 80 0 1 1 156.6 156.6"
                    fill="none"
                    stroke="url(#gradient-1)"
                    strokeWidth="8"
                    strokeLinecap="round"
                    strokeDasharray={`${progress} ${arcLength}`}
                    strokeDashoffset="0"
                    opacity={
                      ledStrips[1]?.state === "ON" && progress > 0 ? 1 : 0
                    }
                  />
                </svg>
              );
            })()}
            <span className="button-text">{ledStrips[1]?.state || "OFF"}</span>
          </div>
        </div>

        {/* Strip 4 - Bedroom (similar to Lighting) */}
        <div
          className="led-card"
          onClick={() => {
            if (socketRef.current) {
              socketRef.current.emit("ledCommand", {
                type: "strip",
                index: 4,
                action: ledStrips[4]?.state === "ON" ? "off" : "on",
              });
            }
          }}
        >
          <p className="led-name">Bedroom</p>
          <div
            className={`neumorphic-button ${
              ledStrips[4]?.state === "ON" ? "on" : "off"
            }`}
          >
            {(() => {
              const arcLength = Math.PI * 80 * (270 / 180);
              const progress = getArcProgress(
                ledStrips[4]?.brightness || 0,
                ledStrips[4]?.state === "ON"
              );
              return (
                <svg className="horseshoe-progress" viewBox="0 0 200 200">
                  <defs>
                    <linearGradient
                      id="gradient-4"
                      x1="0%"
                      y1="0%"
                      x2="0%"
                      y2="100%"
                    >
                      <stop offset="0%" stopColor="#00C6FF" />
                      <stop offset="100%" stopColor="#00FF99" />
                    </linearGradient>
                  </defs>
                  {/* Arc from 135° (start) to 45° (end) - fills according to brightness */}
                  <path
                    className="horseshoe-fill"
                    d="M 43.4 156.6 A 80 80 0 1 1 156.6 156.6"
                    fill="none"
                    stroke="url(#gradient-4)"
                    strokeWidth="8"
                    strokeLinecap="round"
                    strokeDasharray={`${progress} ${arcLength}`}
                    strokeDashoffset="0"
                    opacity={
                      ledStrips[4]?.state === "ON" && progress > 0 ? 1 : 0
                    }
                  />
                </svg>
              );
            })()}
            <span className="button-text">{ledStrips[4]?.state || "OFF"}</span>
          </div>
        </div>

        {/* Strip 3 - Bathroom (motion-activated) with 3-position button */}
        <div
          className="led-card"
          onClick={() => {
            if (socketRef.current) {
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

              socketRef.current.emit("ledCommand", {
                type: "strip",
                index: 3,
                action: "mode",
                value: nextMode,
              });
            }
          }}
        >
          <p className="led-name">Bathroom</p>
          <div
            className={`neumorphic-button ${
              ledStrips[3]?.mode === "ON" || ledStrips[3]?.state === "ON"
                ? "on"
                : ledStrips[3]?.mode === "AUTO"
                ? "auto"
                : "off"
            }`}
          >
            {(() => {
              const arcLength = Math.PI * 80 * (270 / 180);
              const progress = getArcProgress(
                ledStrips[3]?.brightness || 0,
                ledStrips[3]?.state === "ON"
              );
              return (
                <svg className="horseshoe-progress" viewBox="0 0 200 200">
                  <defs>
                    <linearGradient
                      id="gradient-3"
                      x1="0%"
                      y1="0%"
                      x2="0%"
                      y2="100%"
                    >
                      <stop offset="0%" stopColor="#00C6FF" />
                      <stop offset="100%" stopColor="#00FF99" />
                    </linearGradient>
                  </defs>
                  {/* Arc from 135° (start) to 45° (end) - fills according to brightness */}
                  <path
                    className="horseshoe-fill"
                    d="M 43.4 156.6 A 80 80 0 1 1 156.6 156.6"
                    fill="none"
                    stroke="url(#gradient-3)"
                    strokeWidth="8"
                    strokeLinecap="round"
                    strokeDasharray={`${progress} ${arcLength}`}
                    strokeDashoffset="0"
                    opacity={
                      ledStrips[3]?.state === "ON" && progress > 0 ? 1 : 0
                    }
                  />
                </svg>
              );
            })()}
            <span className="button-text">
              {ledStrips[3]?.mode === "AUTO"
                ? "AUTO"
                : ledStrips[3]?.state === "ON"
                ? "ON"
                : "OFF"}
            </span>
          </div>
        </div>

        <div
          className="led-card"
          onClick={() => {
            if (socketRef.current) {
              socketRef.current.emit("ledCommand", {
                type: "relay",
                action: "toggle",
              });
            }
          }}
        >
          <p className="led-name">Floor</p>
          <div
            className={`neumorphic-button ${
              relays[0]?.state === "ON" ? "on" : "off"
            }`}
          >
            <svg className="horseshoe-progress" viewBox="0 0 200 200">
              <defs>
                <linearGradient
                  id="gradient-2"
                  x1="0%"
                  y1="0%"
                  x2="0%"
                  y2="100%"
                >
                  <stop offset="0%" stopColor="#00C6FF" />
                  <stop offset="100%" stopColor="#00FF99" />
                </linearGradient>
              </defs>
              {/* Closed circle - if ON it exists, if OFF it doesn't */}
              {relays[0]?.state === "ON" && (
                <circle
                  className="horseshoe-fill"
                  cx="100"
                  cy="100"
                  r="80"
                  fill="none"
                  stroke="url(#gradient-2)"
                  strokeWidth="8"
                  strokeLinecap="round"
                />
              )}
            </svg>
            <span className="button-text">{relays[0]?.state || "OFF"}</span>
          </div>
        </div>
      </div>
    </div>
  );
}

export default App;
