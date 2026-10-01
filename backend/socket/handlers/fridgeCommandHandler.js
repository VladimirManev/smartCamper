/**
 * Fridge BLE command handler
 * Frontend → MQTT commands for Alpicool fridge on module-6
 *
 * Commands:
 *   { type: "mode", action: "eco"|"max"|"toggle" }
 *   { type: "zone", index: 1|2, action: "set", temp: number }
 */

const fridgeCommandHandler = (socket, aedes, data) => {
  if (!data || !data.type) {
    console.log("❌ Invalid fridge command format");
    return;
  }

  const moduleId = "module-6";
  let mqttTopic;
  let mqttPayload = "{}";

  if (data.type === "mode") {
    const action = data.action;
    if (action !== "eco" && action !== "max" && action !== "toggle") {
      console.log("❌ Invalid fridge mode action:", data);
      return;
    }
    mqttTopic = `smartcamper/commands/${moduleId}/fridge/mode/${action}`;
  } else if (
    data.type === "zone" &&
    (data.index === 1 || data.index === 2) &&
    data.action === "set" &&
    typeof data.temp === "number"
  ) {
    const temp = Math.round(data.temp);
    if (temp < -40 || temp > 40) {
      console.log("❌ Fridge temp out of range:", temp);
      return;
    }
    mqttTopic = `smartcamper/commands/${moduleId}/fridge/zone${data.index}/set`;
    mqttPayload = JSON.stringify({ temp });
  } else {
    console.log("❌ Invalid fridge command:", data);
    return;
  }

  aedes.publish(
    {
      topic: mqttTopic,
      payload: Buffer.from(mqttPayload),
      qos: 0,
    },
    (err) => {
      if (err) {
        console.log(`❌ Failed to publish fridge command: ${err.message}`);
      } else {
        console.log(`📤 Fridge command: ${mqttTopic} = ${mqttPayload}`);
      }
    }
  );
};

module.exports = fridgeCommandHandler;
