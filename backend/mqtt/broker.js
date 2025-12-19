// MQTT Broker Setup
// Aedes MQTT broker configuration

const aedes = require("aedes")();
const net = require("net");

const setupMQTTBroker = () => {
  // Start MQTT broker on port 1883
  const mqttServer = net.createServer(aedes.handle);

  mqttServer.listen(1883, () => {
    console.log(`🔌 MQTT Broker (Aedes) running on port 1883`);
  });

  // MQTT broker events
  aedes.on("client", (client) => {
    console.log(`📱 MQTT client connected: ${client.id}`);
  });

  aedes.on("clientDisconnect", (client) => {
    console.log(`📱 MQTT client disconnected: ${client.id}`);
  });

  aedes.on("publish", (packet, client) => {
    if (client) {
      console.log(
        `📨 MQTT publish: ${packet.topic} = ${packet.payload.toString()}`
      );
    }
  });

  return aedes;
};

module.exports = setupMQTTBroker;
