import { useState, useEffect } from "react";
import io from "socket.io-client";
import "./App.css";

function App() {
  // State за температура и влажност
  const [temperature, setTemperature] = useState(null);
  const [humidity, setHumidity] = useState(null);
  const [connected, setConnected] = useState(false);

  // useEffect = изпълнява се когато компонентът се зареди
  useEffect(() => {
    // Свързваме се с backend WebSocket
    const socket = io("http://localhost:3000");

    // Когато се свържем
    socket.on("connect", () => {
      console.log("✅ Свързан с backend");
      setConnected(true);
    });

    // Когато се изключим
    socket.on("disconnect", () => {
      console.log("❌ Изключен от backend");
      setConnected(false);
    });

    // Слушаме за обновления на сензорите
    socket.on("sensorUpdate", (data) => {
      console.log("📊 Нови данни:", data);
      setTemperature(data.temperature);
      setHumidity(data.humidity);
    });

    // Cleanup функция - изключва socket когато компонентът се unmount-не
    return () => {
      socket.disconnect();
    };
  }, []); // [] = изпълни само веднъж при зареждане

  return (
    <div className="app">
      <h1>🚐 SmartCamper Dashboard</h1>

      <div className="status">
        <p>Статус: {connected ? "Онлайн ✅" : "Офлайн ❌"}</p>
      </div>

      <div className="sensor-card">
        <h2>🌡️ Температура</h2>
        <p className="value">
          {temperature !== null ? `${temperature}°C` : "Зарежда..."}
        </p>
      </div>

      <div className="sensor-card">
        <h2>💧 Влажност</h2>
        <p className="value">
          {humidity !== null ? `${humidity}%` : "Зарежда..."}
        </p>
      </div>
    </div>
  );
}

export default App;
