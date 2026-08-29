/**
 * Security Command Handler
 * Forwards frontend security commands to module-8 over MQTT.
 */

const MODULE_ID = "module-8";

const securityCommandHandler = (socket, aedes, data) => {
  if (!data || (data.zone !== 1 && data.zone !== 2)) {
    console.log("❌ Invalid security command format");
    return;
  }

  if (data.action !== "on" && data.action !== "off") {
    // simulate_* is mock-only; ignore on real hardware bridge
    if (
      data.action === "simulate_motion" ||
      data.action === "simulate_trip"
    ) {
      return;
    }
    console.log("❌ Invalid security command action:", data.action);
    return;
  }

  const mqttTopic = `smartcamper/commands/${MODULE_ID}/zone/${data.zone}/${data.action}`;
  let mqttPayload = "{}";

  if (data.zone === 1 && data.action === "on") {
    mqttPayload = JSON.stringify({
      ignoreInteriorPir: !!data.ignoreInteriorPir,
    });
  } else if (data.zone === 1 && data.action === "off") {
    mqttPayload = JSON.stringify({
      pin: String(data.pin || ""),
    });
  }

  aedes.publish(
    {
      topic: mqttTopic,
      payload: Buffer.from(mqttPayload),
      qos: 0,
    },
    (err) => {
      if (err) {
        console.log(`❌ Failed to publish security command: ${err.message}`);
      } else {
        console.log(
          `📤 Published security command: ${mqttTopic} = ${mqttPayload}`
        );
      }
    }
  );
};

module.exports = securityCommandHandler;
