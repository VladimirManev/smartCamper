/**
 * LED Command Handler
 * Handles LED commands from frontend and publishes to MQTT
 */

const ledCommandHandler = (socket, aedes, data) => {
  console.log("💡 LED Command received:", data);

  // Validate data
  if (!data || !data.type) {
    console.log("❌ Invalid LED command format");
    return;
  }

  // Use module-2 as the module ID (led-controller was renamed to module-2)
  const moduleId = "module-2";

  let mqttTopic;
  let mqttPayload = "{}";

  // Construct MQTT topic and payload according to command type
  if (
    data.type === "strip" &&
    typeof data.index === "number" &&
    data.action
  ) {
    // Strip command: strip/{index}/{action}
    mqttTopic = `smartcamper/commands/${moduleId}/strip/${data.index}/${data.action}`;

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
    mqttTopic = `smartcamper/commands/${moduleId}/relay/toggle`;
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
};

module.exports = ledCommandHandler;

