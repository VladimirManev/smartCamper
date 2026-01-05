/**
 * Floor Heating Command Handler
 * Handles floor heating commands from frontend and publishes to MQTT
 */

const floorHeatingCommandHandler = (socket, aedes, data) => {
  console.log("🔥 Floor Heating Command received:", data);

  // Validate data
  if (!data || !data.type) {
    console.log("❌ Invalid floor heating command format");
    return;
  }

  // Use module-3 as the module ID
  const moduleId = "module-3";

  let mqttTopic;
  let mqttPayload = "{}";

  // Construct MQTT topic and payload according to command type
  if (
    data.type === "circle" &&
    typeof data.index === "number" &&
    data.action
  ) {
    // Circle command: circle/{index}/{action}
    mqttTopic = `smartcamper/commands/${moduleId}/circle/${data.index}/${data.action}`;

    // If value exists, add it to payload (for future use)
    if (data.value !== undefined) {
      mqttPayload = JSON.stringify({ value: data.value });
    }
  } else {
    console.log("❌ Invalid floor heating command:", data);
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
        console.log(`❌ Failed to publish floor heating command: ${err.message}`);
      } else {
        console.log(
          `📤 Published floor heating command: ${mqttTopic} = ${mqttPayload}`
        );
      }
    }
  );
};

module.exports = floorHeatingCommandHandler;

