const aedes = require("aedes")();
const net = require("net");

// Създаваме MQTT сървър
const mqttServer = net.createServer(aedes.handle);

// Порт за MQTT (стандартен е 1883)
const MQTT_PORT = 1883;

// Съхраняваме данните в памет (по-късно ще добавим MongoDB)
const sensorData = {
  temperature: {},
  humidity: {},
  waterLevel: {},
  battery: {},
  tilt: {},
};

// Когато клиент се свърже
aedes.on("client", (client) => {
  console.log(`📡 MQTT клиент се свърза: ${client.id}`);
});

// Когато клиент се отключи
aedes.on("clientDisconnect", (client) => {
  console.log(`📡 MQTT клиент се отключи: ${client.id}`);
});

// Когато получаваме съобщение
aedes.on("publish", (packet, client) => {
  if (client) {
    console.log(`📨 Получено MQTT съобщение от ${client.id}: ${packet.topic}`);

    // Обработваме различните типове данни
    handleSensorData(packet.topic, packet.payload.toString());
  }
});

// Функция за обработка на данни от сензори
function handleSensorData(topic, payload) {
  try {
    const data = JSON.parse(payload);

    // Разделяме topic-а на части
    const parts = topic.split("/");

    if (parts[0] === "smartcamper" && parts[1] === "sensors") {
      const sensorType = parts[2];
      const deviceId = parts[3];

      // Запазваме данните
      if (!sensorData[sensorType]) {
        sensorData[sensorType] = {};
      }

      sensorData[sensorType][deviceId] = {
        ...data,
        timestamp: new Date().toISOString(),
      };

      console.log(
        `💾 Запазени данни: ${sensorType}/${deviceId} = ${data.value}${
          data.unit || ""
        }`
      );
    }
  } catch (error) {
    console.error("❌ Грешка при обработка на MQTT данни:", error);
  }
}

// Функция за получаване на данни (за API)
function getSensorData(sensorType, deviceId) {
  if (deviceId) {
    return sensorData[sensorType]?.[deviceId] || null;
  }
  return sensorData[sensorType] || {};
}

// Стартираме MQTT сървъра
mqttServer.listen(MQTT_PORT, () => {
  console.log(`📡 MQTT Broker стартиран на порт ${MQTT_PORT}`);
});

module.exports = {
  aedes,
  getSensorData,
  handleSensorData,
};
