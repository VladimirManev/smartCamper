/**
 * Damper Command Handler
 * Handles damper commands from frontend and publishes to MQTT
 */

const damperCommandHandler = (socket, aedes, data) => {
  console.log("🌬️ Damper Command received:", data);

  // Validate data
  if (!data || !data.type || data.type !== "damper") {
    console.log("❌ Invalid damper command format");
    return;
  }

  // Use module-4 as the module ID
  const moduleId = "module-4";

  // Validate required fields
  if (typeof data.index !== "number" || !data.action) {
    console.log("❌ Invalid damper command: missing index or action");
    return;
  }

  let mqttTopic;
  let mqttPayload = "{}";

  // Construct MQTT topic and payload
  if (data.action === "set_angle" && typeof data.angle === "number") {
    // Damper command: damper/{index}/set_angle
    mqttTopic = `smartcamper/commands/${moduleId}/damper/${data.index}/set_angle`;
    mqttPayload = JSON.stringify({
      type: "damper",
      index: data.index,
      action: "set_angle",
      angle: data.angle,
    });
  } else {
    console.log("❌ Invalid damper command action:", data.action);
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
        console.log(`❌ Failed to publish damper command: ${err.message}`);
      } else {
        console.log(
          `📤 Published damper command: ${mqttTopic} = ${mqttPayload}`
        );
      }
    }
  );
};

module.exports = damperCommandHandler;

