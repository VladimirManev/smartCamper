import { useState, useEffect } from "react";
import io from "socket.io-client";
import "./App.css";

function App() {
  // State за температура и влажност
  const [temperature, setTemperature] = useState(null);
  const [humidity, setHumidity] = useState(null);
  const [connected, setConnected] = useState(false);
  const [esp32Connected, setEsp32Connected] = useState(false);

  // useEffect = изпълнява се когато компонентът се зареди
  useEffect(() => {
    // Свързваме се с backend WebSocket
    const socket = io("http://localhost:3000");

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

    // НЕ стартираме timeout веднага - иконата трябва да е червена до получаване на данни

    // Cleanup функция - изключва socket когато компонентът се unmount-не
    return () => {
      socket.disconnect();
      clearTimeout(esp32Timeout);
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
      </div>

      <div className="sensor-cards">
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
      </div>
    </div>
  );
}

export default App;
