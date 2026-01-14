/**
 * Leveling Command Handler
 * Handles leveling commands from frontend and publishes to MQTT
 */

const levelingCommandHandler = (socket, aedes, data) => {
  console.log("📐 Leveling Command received:", data);

  // Validate data
  if (!data || data.type !== "start") {
    console.log("❌ Invalid leveling command format");
    return;
  }

  // Use module-3 as the module ID
  const moduleId = "module-3";

  // Construct MQTT topic
  const mqttTopic = `smartcamper/commands/${moduleId}/leveling/start`;
  const mqttPayload = "{}";

  // Publish command to MQTT
  aedes.publish(
    {
      topic: mqttTopic,
      payload: Buffer.from(mqttPayload),
      qos: 0,
    },
    (err) => {
      if (err) {
        console.log(`❌ Failed to publish leveling command: ${err.message}`);
      } else {
        if (process.env.DEBUG_MQTT) {
          console.log(
            `📤 Published leveling command: ${mqttTopic} = ${mqttPayload}`
          );
        }
      }
    }
  );
};

module.exports = levelingCommandHandler;
