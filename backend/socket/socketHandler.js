// Socket.io Handler + MQTT Bridge
// Обработка на WebSocket комуникация и MQTT ↔ WebSocket bridge

const setupSocketIO = (io, aedes) => {
  // Съхраняваме текущите данни от сензорите
  let sensorData = {
    temperature: null,
    humidity: null,
    timestamp: null,
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

      // Парсираме стойността според типа сензор
      let value;
      if (sensorType === "temperature") {
        value = parseFloat(message);
      } else if (sensorType === "humidity") {
        value = parseInt(message); // Влажността е цяло число
      }

      // Проверяваме дали стойността е валидна
      if (!isNaN(value)) {
        // Обновяваме данните
        if (sensorType === "temperature") {
          sensorData.temperature = value;
        } else if (sensorType === "humidity") {
          sensorData.humidity = value;
        }

        sensorData.timestamp = new Date().toISOString();

        // Bridge: MQTT → WebSocket
        io.emit("sensorUpdate", sensorData);
      }
    }
  });

  // WebSocket connection events
  io.on("connection", (socket) => {
    console.log("✅ Frontend се свърза с WebSocket");

    // НЕ изпращаме стари данни - frontend ще получи данни само при нови MQTT съобщения
    // Това гарантира, че иконите започват червени и стават зелени само при реални данни

    // Когато frontend се изключи
    socket.on("disconnect", () => {
      console.log("❌ Frontend се изключи");
    });
  });
};

module.exports = setupSocketIO;
