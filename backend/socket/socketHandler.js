// Socket.io Handler + MQTT Bridge
// Обработка на WebSocket комуникация и MQTT ↔ WebSocket bridge

const setupSocketIO = (io, aedes) => {
  // Съхраняваме текущите данни от сензорите
  let sensorData = {
    temperature: null,
    humidity: null,
    timestamp: null,
  };

  // Health check за модулите
  let moduleLastSeen = {
    "temperature-sensor": null,
    "water-sensor": null,
    // Добави други модули тук
  };

  // MQTT ↔ WebSocket Bridge - слушаме Aedes broker директно
  aedes.on("publish", (packet, client) => {
    const topic = packet.topic;
    const message = packet.payload.toString();

    console.log(`📨 MQTT: ${topic} = ${message}`);

    // Проверяваме дали е сензорен топик
    if (topic.startsWith("smartcamper/sensors/")) {
      const topicParts = topic.split("/");
      const sensorType = topicParts[2]; // smartcamper/sensors/temperature
      const value = parseFloat(message);

      // Обновяваме данните
      if (sensorType === "temperature") {
        sensorData.temperature = value;
        moduleLastSeen["temperature-sensor"] = Date.now();
      } else if (sensorType === "humidity") {
        sensorData.humidity = value;
        moduleLastSeen["temperature-sensor"] = Date.now();
      }

      sensorData.timestamp = new Date().toISOString();

      // Bridge: MQTT → WebSocket
      io.emit("sensorUpdate", sensorData);
    }
  });

  // WebSocket connection events
  io.on("connection", (socket) => {
    console.log("✅ Frontend се свърза с WebSocket");

    // Изпращаме текущите данни (ако има)
    if (sensorData.temperature !== null) {
      socket.emit("sensorUpdate", sensorData);
    } else {
      // Ако няма реални данни, изпращаме примерни
      socket.emit("sensorUpdate", {
        temperature: 25.5,
        humidity: 60,
        timestamp: new Date().toISOString(),
      });
    }

    // Когато frontend се изключи
    socket.on("disconnect", () => {
      console.log("❌ Frontend се изключи");
    });
  });

  // Health Check - проверяваме модулите на всеки 5 секунди
  setInterval(() => {
    const now = Date.now();
    const HEALTH_CHECK_TIMEOUT = 10000; // 10 секунди

    Object.keys(moduleLastSeen).forEach((moduleId) => {
      const lastSeen = moduleLastSeen[moduleId];
      const timeSinceLastMessage = now - lastSeen;

      if (lastSeen === null) {
        // Модулът никога не е пращал съобщение
        console.log(`⚠️ Module ${moduleId} never sent data`);
        forceUpdateModule(moduleId);
      } else if (timeSinceLastMessage > HEALTH_CHECK_TIMEOUT) {
        // Модулът не е пращал съобщение за 10+ секунди
        console.log(
          `⚠️ Module ${moduleId} offline for ${Math.round(
            timeSinceLastMessage / 1000
          )}s - forcing update`
        );
        forceUpdateModule(moduleId);
      } else {
        // Модулът е активен
        console.log(
          `✅ Module ${moduleId} is healthy (last seen ${Math.round(
            timeSinceLastMessage / 1000
          )}s ago)`
        );
      }
    });
  }, 5000); // Проверяваме на всеки 5 секунди

  // Функция за force update на модул
  function forceUpdateModule(moduleId) {
    const topic = `smartcamper/commands/${moduleId}/force_update`;
    const message = "ping";

    aedes.publish(
      {
        topic: topic,
        payload: message,
        qos: 0,
        retain: false,
      },
      () => {
        console.log(`🔄 Force update sent to ${moduleId}`);
      }
    );
  }
};

module.exports = setupSocketIO;
