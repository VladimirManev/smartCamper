import { useState, useEffect, useRef } from "react";
import io from "socket.io-client";
import "./App.css";

function App() {
  // State за температура и влажност
  const [temperature, setTemperature] = useState(null);
  const [humidity, setHumidity] = useState(null);
  const [connected, setConnected] = useState(false);
  const [esp32Connected, setEsp32Connected] = useState(false);

  // State за LED контролер
  const [ledControllerConnected, setLedControllerConnected] = useState(false);
  const [ledStrips, setLedStrips] = useState({
    0: { state: "OFF", brightness: 0 }, // Kitchen
    1: { state: "OFF", brightness: 0 }, // Lighting
  });
  const [relays, setRelays] = useState({
    0: { state: "OFF" }, // Relay 0
  });

  // Запазваме socket референция за използване в бутоните
  const socketRef = useRef(null);

  // Helper функция за изчисляване на прогреса на дъгата
  // Дъгата е от 135° до 45° = 270 градуса общо
  // Запълва се спрямо brightness стойността
  const getArcProgress = (brightness, isOn) => {
    if (!isOn || brightness === 0) {
      return 0;
    }
    // Дължина на дъгата: 270 градуса = π * radius * (270/180) ≈ 377px (за радиус 80)
    const arcLength = Math.PI * 80 * (270 / 180);

    // Прогрес от 0 до 1 спрямо brightness (0-255)
    const progress = brightness / 255;

    return progress * arcLength;
  };

  // useEffect = изпълнява се когато компонентът се зареди
  useEffect(() => {
    // Свързваме се с backend WebSocket
    // В development: използваме Raspberry Pi IP
    // В production: използваме същия host (relative URL)
    const isDevelopment = import.meta.env.DEV;
    const socketUrl = isDevelopment
      ? "http://192.168.4.1:3000" // Raspberry Pi IP
      : window.location.origin; // Production - същия host
    const socket = io(socketUrl);
    socketRef.current = socket; // Запазваме референцията

    // Когато се свържем
    socket.on("connect", () => {
      console.log("✅ Свързан с backend");
      setConnected(true);
      // ESP32 статусът остава false докато не получим данни
      setEsp32Connected(false); // Принудително reset ESP32 статуса
    });

    // Когато се изключим
    socket.on("disconnect", () => {
      console.log("❌ Изключен от backend");
      setConnected(false);
      setEsp32Connected(false); // Reset ESP32 status on backend disconnect
    });

    // Timeout за ESP32 - глобална променлива
    let esp32Timeout;
    let ledControllerTimeout;

    // Слушаме за обновления на сензорите
    socket.on("sensorUpdate", (data) => {
      console.log("📊 Нови данни:", data);
      setTemperature(data.temperature);
      setHumidity(data.humidity);
      setEsp32Connected(true);

      // Рестартираме timeout-а всеки път когато получаваме данни
      clearTimeout(esp32Timeout);
      esp32Timeout = setTimeout(() => {
        setEsp32Connected(false);
        setTemperature(null); // Изчистваме температурата
        setHumidity(null); // Изчистваме влажността
      }, 30000); // 30 секунди timeout (20 секунди резерв след ESP32 heartbeat)
    });

    // Heartbeat вече се обработва чрез ledStatusUpdate (пълен статус на интервали)

    // Слушаме за LED статус обновления
    socket.on("ledStatusUpdate", (data) => {
      console.log("💡 LED Status Update:", data);

      // Всяко статус обновление означава че модулът е жив - обновяваме heartbeat
      setLedControllerConnected(true);
      clearTimeout(ledControllerTimeout);
      ledControllerTimeout = setTimeout(() => {
        setLedControllerConnected(false);
      }, 30000); // 30 секунди timeout

      // НОВ ФОРМАТ: Пълен статус в един обект
      if (data.type === "full" && data.data) {
        const statusData = data.data;
        
        // Обновяваме всички ленти
        if (statusData.strips) {
          const newStrips = {};
          for (const [index, stripData] of Object.entries(statusData.strips)) {
            newStrips[index] = {
              state: stripData.state,
              brightness: stripData.brightness,
            };
          }
          setLedStrips(newStrips);
        }
        
        // Обновяваме всички релета (формат като лентите)
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
      // СТАР ФОРМАТ (за обратна съвместимост)
      else if (data.type === "strip" && typeof data.index === "number") {
        setLedStrips((prev) => ({
          ...prev,
          [data.index]: {
            ...prev[data.index],
            [data.dataType]: data.value,
          },
        }));
      } else if (data.type === "relay") {
        // Стар формат - обновяваме relay 0
        setRelays((prev) => ({
          ...prev,
          0: { state: data.value },
        }));
      }
    });

    // НЕ стартираме timeout веднага - иконите трябва да са червени до получаване на данни

    // Cleanup функция - изключва socket когато компонентът се unmount-не
    return () => {
      socket.disconnect();
      clearTimeout(esp32Timeout);
      clearTimeout(ledControllerTimeout);
    };
  }, []); // [] = изпълни само веднъж при зареждане

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
                  {/* Дъга от 135° (начало) до 45° (край) - запълва се спрямо brightness */}
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
                  {/* Дъга от 135° (начало) до 45° (край) - запълва се спрямо brightness */}
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
              {/* Затворена окръжност - ако е ON я има, ако е OFF я няма */}
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
