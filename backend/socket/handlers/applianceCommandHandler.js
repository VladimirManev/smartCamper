/**
 * Appliance Command Handler
 * Handles appliance commands from frontend and publishes to MQTT
 */

const applianceCommandHandler = (socket, aedes, data) => {
  console.log("🔌 Appliance Command received:", data);

  // Validate data
  if (!data || !data.type) {
    console.log("❌ Invalid appliance command format");
    return;
  }

  // Use module-5 as the module ID
  const moduleId = "module-5";

  let mqttTopic;
  let mqttPayload = "{}";

  // Construct MQTT topic and payload according to command type
  if (
    data.type === "relay" &&
    typeof data.index === "number" &&
    data.action === "toggle"
  ) {
    // Relay command: relay/{index}/toggle
    mqttTopic = `smartcamper/commands/${moduleId}/relay/${data.index}/toggle`;
  } else {
    console.log("❌ Invalid appliance command:", data);
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
        console.log(`❌ Failed to publish appliance command: ${err.message}`);
      } else {
        console.log(
          `📤 Published appliance command: ${mqttTopic} = ${mqttPayload}`
        );
      }
    }
  );
};

module.exports = applianceCommandHandler;
