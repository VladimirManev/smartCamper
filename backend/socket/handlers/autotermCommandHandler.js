/**
 * Autoterm Command Handler
 * Forwards by-power start / set_power / stop to module-4 over MQTT
 */

const autotermCommandHandler = (socket, aedes, data) => {
  console.log("🔥 Autoterm Command received:", data);

  if (!data || data.type !== "autoterm" || !data.action) {
    console.log("❌ Invalid autoterm command format");
    return;
  }

  const moduleId = "module-4";
  const action = data.action;
  let mqttTopic;
  let mqttPayload;

  if (action === "stop") {
    mqttTopic = `smartcamper/commands/${moduleId}/autoterm/stop`;
    mqttPayload = JSON.stringify({ type: "autoterm", action: "stop" });
  } else if (action === "start") {
    const power = typeof data.power === "number" ? data.power : 5;
    if (power < 1 || power > 9) {
      console.log("❌ Invalid autoterm start power:", power);
      return;
    }
    mqttTopic = `smartcamper/commands/${moduleId}/autoterm/start`;
    mqttPayload = JSON.stringify({
      type: "autoterm",
      action: "start",
      power,
    });
  } else if (action === "set_power") {
    if (typeof data.power !== "number" || data.power < 1 || data.power > 9) {
      console.log("❌ Invalid autoterm set_power:", data.power);
      return;
    }
    mqttTopic = `smartcamper/commands/${moduleId}/autoterm/set_power`;
    mqttPayload = JSON.stringify({
      type: "autoterm",
      action: "set_power",
      power: data.power,
    });
  } else {
    console.log("❌ Invalid autoterm action:", action);
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
        console.log(`❌ Failed to publish autoterm command: ${err.message}`);
      } else {
        console.log(
          `📤 Published autoterm command: ${mqttTopic} = ${mqttPayload}`
        );
      }
    }
  );
};

module.exports = autotermCommandHandler;
