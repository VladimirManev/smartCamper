/**
 * Appliance Command Handler
 * Handles appliance commands from frontend and publishes to MQTT
 *
 * Fridge power (index 2) is on module-6; other relays stay on module-5.
 */

const FRIDGE_RELAY_INDEX = 2;

const applianceCommandHandler = (socket, aedes, data) => {
  console.log("🔌 Appliance Command received:", data);

  if (!data || !data.type) {
    console.log("❌ Invalid appliance command format");
    return;
  }

  let mqttTopic;
  let mqttPayload = "{}";

  const relayActions = ["toggle", "on", "off"];
  if (
    data.type === "relay" &&
    typeof data.index === "number" &&
    relayActions.includes(data.action)
  ) {
    if (data.index === FRIDGE_RELAY_INDEX) {
      mqttTopic = `smartcamper/commands/module-6/fridge/power/${data.action}`;
    } else {
      mqttTopic = `smartcamper/commands/module-5/relay/${data.index}/${data.action}`;
    }
  } else {
    console.log("❌ Invalid appliance command:", data);
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
