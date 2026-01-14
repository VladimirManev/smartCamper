/**
 * Table Command Handler
 * Handles table commands from frontend and publishes to MQTT
 */

const tableCommandHandler = (socket, aedes, data) => {
  console.log("🪑 Table Command received:", data);

  // Validate data
  if (!data || !data.type || data.type !== "table") {
    console.log("❌ Invalid table command format");
    return;
  }

  // Use module-4 as the module ID
  const moduleId = "module-4";

  // Validate required fields
  if (!data.action) {
    console.log("❌ Invalid table command: missing action");
    return;
  }

  let mqttTopic;
  let mqttPayload = "{}";

  // Construct MQTT topic and payload based on action
  if (data.action === "move_up") {
    mqttTopic = `smartcamper/commands/${moduleId}/table/move_up`;
    mqttPayload = JSON.stringify({
      type: "table",
      action: "move_up",
    });
  } else if (data.action === "move_down") {
    mqttTopic = `smartcamper/commands/${moduleId}/table/move_down`;
    mqttPayload = JSON.stringify({
      type: "table",
      action: "move_down",
    });
  } else if (data.action === "stop") {
    mqttTopic = `smartcamper/commands/${moduleId}/table/stop`;
    mqttPayload = JSON.stringify({
      type: "table",
      action: "stop",
    });
  } else if (data.action === "move_up_auto") {
    mqttTopic = `smartcamper/commands/${moduleId}/table/move_up_auto`;
    mqttPayload = JSON.stringify({
      type: "table",
      action: "move_up_auto",
      // duration премахнато - ESP32 използва своята константа TABLE_AUTO_MOVE_DURATION
    });
  } else if (data.action === "move_down_auto") {
    mqttTopic = `smartcamper/commands/${moduleId}/table/move_down_auto`;
    mqttPayload = JSON.stringify({
      type: "table",
      action: "move_down_auto",
      // duration премахнато - ESP32 използва своята константа TABLE_AUTO_MOVE_DURATION
    });
  } else {
    console.log("❌ Invalid table command action:", data.action);
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
        console.log(`❌ Failed to publish table command: ${err.message}`);
      } else {
        console.log(
          `📤 Published table command: ${mqttTopic} = ${mqttPayload}`
        );
      }
    }
  );
};

module.exports = tableCommandHandler;

