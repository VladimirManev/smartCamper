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

      // Обработка на температурен сензор
      if (sensorType === "temperature" || sensorType === "humidity") {
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
      // Обработка на LED контролер данни
      else if (sensorType === "led-controller") {
        // НОВ ФОРМАТ: smartcamper/sensors/led-controller/status (JSON с всички данни)
        // СТАР ФОРМАТ (за обратна съвместимост): smartcamper/sensors/led-controller/strip/{index}/state
        // или: smartcamper/sensors/led-controller/strip/{index}/brightness
        // или: smartcamper/sensors/led-controller/relay/state

        if (topicParts.length >= 4) {
          const subType = topicParts[3]; // status, strip, или relay

          // НОВ ФОРМАТ: Пълен статус в един JSON обект (включва heartbeat)
          if (subType === "status") {
            try {
              const statusData = JSON.parse(message);
              
              // Изпращаме пълния статус на frontend
              io.emit("ledStatusUpdate", {
                type: "full",
                data: statusData,
                timestamp: new Date().toISOString(),
              });
            } catch (error) {
              console.log(`❌ Failed to parse LED status JSON: ${error.message}`);
            }
          } 
          // СТАР ФОРМАТ (за обратна съвместимост - може да се премахне в бъдеще)
          else if (subType === "strip" && topicParts.length >= 6) {
            // Strip данни: smartcamper/sensors/led-controller/strip/{index}/{type}
            const stripIndex = parseInt(topicParts[4]);
            const dataType = topicParts[5]; // state или brightness

            if (!isNaN(stripIndex) && (dataType === "state" || dataType === "brightness")) {
              io.emit("ledStatusUpdate", {
                type: "strip",
                index: stripIndex,
                dataType: dataType,
                value: dataType === "brightness" ? parseInt(message) : message,
                timestamp: new Date().toISOString(),
              });
            }
          } else if (subType === "relay" && topicParts.length >= 5) {
            // Relay данни: smartcamper/sensors/led-controller/relay/state
            const dataType = topicParts[4]; // state

            if (dataType === "state") {
              io.emit("ledStatusUpdate", {
                type: "relay",
                dataType: "state",
                value: message,
                timestamp: new Date().toISOString(),
              });
            }
          }
        }
      }
    }
  });

  // WebSocket connection events
  io.on("connection", (socket) => {
    console.log("✅ Frontend се свърза с WebSocket");

    // НЕ изпращаме стари данни - frontend ще получи данни само при нови MQTT съобщения
    // Това гарантира, че иконите започват червени и стават зелени само при реални данни

    // Обработка на LED команди от frontend
    socket.on("ledCommand", (data) => {
      console.log("💡 LED Command received:", data);

      // Валидираме данните
      if (!data || !data.type) {
        console.log("❌ Invalid LED command format");
        return;
      }

      let mqttTopic;
      let mqttPayload = "{}";

      // Конструираме MQTT topic и payload според типа команда
      if (data.type === "strip" && typeof data.index === "number" && data.action) {
        // Strip команда: strip/{index}/{action}
        mqttTopic = `smartcamper/commands/led-controller/strip/${data.index}/${data.action}`;

        // Ако има brightness стойност, добавяме я в payload
        if (data.action === "brightness" && typeof data.value === "number") {
          mqttPayload = JSON.stringify({ value: data.value });
        }
      } else if (data.type === "relay" && data.action === "toggle") {
        // Relay команда: relay/toggle
        mqttTopic = `smartcamper/commands/led-controller/relay/toggle`;
      } else {
        console.log("❌ Invalid LED command:", data);
        return;
      }

      // Публикуваме командата към MQTT
      aedes.publish(
        {
          topic: mqttTopic,
          payload: Buffer.from(mqttPayload),
          qos: 0,
        },
        (err) => {
          if (err) {
            console.log(`❌ Failed to publish LED command: ${err.message}`);
          } else {
            console.log(`📤 Published LED command: ${mqttTopic} = ${mqttPayload}`);
          }
        }
      );
    });

    // Когато frontend се изключи
    socket.on("disconnect", () => {
      console.log("❌ Frontend се изключи");
    });
  });
};

module.exports = setupSocketIO;
