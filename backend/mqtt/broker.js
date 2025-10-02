// MQTT Broker Setup
// Aedes MQTT broker конфигурация

const aedes = require("aedes")();
const net = require("net");

const setupMQTTBroker = () => {
  // Стартираме MQTT broker на порт 1883
  const mqttServer = net.createServer(aedes.handle);

  mqttServer.listen(1883, () => {
    console.log(`🔌 MQTT Broker (Aedes) running on port 1883`);
  });

  // MQTT broker events
  aedes.on("client", (client) => {
    console.log(`📱 MQTT клиент се свърза: ${client.id}`);
  });

  aedes.on("clientDisconnect", (client) => {
    console.log(`📱 MQTT клиент се изключи: ${client.id}`);
  });

  aedes.on("publish", (packet, client) => {
    if (client) {
      console.log(
        `📨 MQTT публикуване: ${packet.topic} = ${packet.payload.toString()}`
      );
    }
  });

  return aedes;
};

module.exports = setupMQTTBroker;
