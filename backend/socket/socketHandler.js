// Socket.io Handler + MQTT Bridge
// WebSocket communication handling and MQTT ↔ WebSocket bridge

const setupSocketIO = (io, aedes) => {
  // Store current sensor data
  let sensorData = {
    temperature: null,
    humidity: null,
    timestamp: null,
  };

  // MQTT ↔ WebSocket Bridge - listen to Aedes broker directly
  aedes.on("publish", (packet, client) => {
    const topic = packet.topic;
    const message = packet.payload.toString();

    console.log(`📨 MQTT: ${topic} = ${message}`);

    // Check if it's a sensor topic
    if (topic.startsWith("smartcamper/sensors/")) {
      const topicParts = topic.split("/");
      const sensorType = topicParts[2]; // smartcamper/sensors/temperature

      // Handle temperature sensor
      if (sensorType === "temperature" || sensorType === "humidity") {
        // Parse value according to sensor type
        let value;
        if (sensorType === "temperature") {
          value = parseFloat(message);
        } else if (sensorType === "humidity") {
          value = parseInt(message); // Humidity is an integer
        }

        // Check if value is valid
        if (!isNaN(value)) {
          // Update data
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
      // Handle LED controller data
      else if (sensorType === "led-controller") {
        // NEW FORMAT: smartcamper/sensors/led-controller/status (JSON with all data)
        // OLD FORMAT (for backward compatibility): smartcamper/sensors/led-controller/strip/{index}/state
        // or: smartcamper/sensors/led-controller/strip/{index}/brightness
        // or: smartcamper/sensors/led-controller/relay/state

        if (topicParts.length >= 4) {
          const subType = topicParts[3]; // status, strip, or relay

          // NEW FORMAT: Full status in one JSON object (includes heartbeat)
          if (subType === "status") {
            try {
              const statusData = JSON.parse(message);

              // Send full status to frontend
              io.emit("ledStatusUpdate", {
                type: "full",
                data: statusData,
                timestamp: new Date().toISOString(),
              });
            } catch (error) {
              console.log(
                `❌ Failed to parse LED status JSON: ${error.message}`
              );
            }
          }
          // OLD FORMAT (for backward compatibility - may be removed in the future)
          else if (subType === "strip" && topicParts.length >= 6) {
            // Strip data: smartcamper/sensors/led-controller/strip/{index}/{type}
            const stripIndex = parseInt(topicParts[4]);
            const dataType = topicParts[5]; // state or brightness

            if (
              !isNaN(stripIndex) &&
              (dataType === "state" || dataType === "brightness")
            ) {
              io.emit("ledStatusUpdate", {
                type: "strip",
                index: stripIndex,
                dataType: dataType,
                value: dataType === "brightness" ? parseInt(message) : message,
                timestamp: new Date().toISOString(),
              });
            }
          } else if (subType === "relay" && topicParts.length >= 5) {
            // Relay data: smartcamper/sensors/led-controller/relay/state
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
    console.log("✅ Frontend connected via WebSocket");

    // Do NOT send old data - frontend will receive data only on new MQTT messages
    // This ensures icons start red and turn green only on real data

    // Handle LED commands from frontend
    socket.on("ledCommand", (data) => {
      console.log("💡 LED Command received:", data);

      // Validate data
      if (!data || !data.type) {
        console.log("❌ Invalid LED command format");
        return;
      }

      let mqttTopic;
      let mqttPayload = "{}";

      // Construct MQTT topic and payload according to command type
      if (
        data.type === "strip" &&
        typeof data.index === "number" &&
        data.action
      ) {
        // Strip command: strip/{index}/{action}
        mqttTopic = `smartcamper/commands/led-controller/strip/${data.index}/${data.action}`;

        // If brightness value exists, add it to payload
        if (data.action === "brightness" && typeof data.value === "number") {
          mqttPayload = JSON.stringify({ value: data.value });
        }
        // If mode value exists, add it to payload
        else if (data.action === "mode" && data.value) {
          mqttPayload = JSON.stringify({ mode: data.value });
        }
      } else if (data.type === "relay" && data.action === "toggle") {
        // Relay command: relay/toggle
        mqttTopic = `smartcamper/commands/led-controller/relay/toggle`;
      } else {
        console.log("❌ Invalid LED command:", data);
        return;
      }

      // Publish command to MQTT
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
            console.log(
              `📤 Published LED command: ${mqttTopic} = ${mqttPayload}`
            );
          }
        }
      );
    });

    // When frontend disconnects
    socket.on("disconnect", () => {
      console.log("❌ Frontend disconnected");
    });
  });
};

module.exports = setupSocketIO;
