/**
 * Module Command Handler
 * Handles sending commands to ESP32 modules via MQTT
 * Provides centralized command sending functionality
 */

/**
 * Send force update command to a specific module
 * @param {Object} aedes - Aedes MQTT broker instance
 * @param {string} moduleId - Module identifier (e.g., "module-1")
 * @returns {Promise<boolean>} True if command was sent successfully
 */
const sendForceUpdate = (aedes, moduleId) => {
  return new Promise((resolve) => {
    const topic = `smartcamper/commands/${moduleId}/force_update`;
    const payload = Buffer.from("{}");

    console.log(`📤 Sending force_update command to ${moduleId} on topic: ${topic}`);

    aedes.publish(
      {
        topic: topic,
        payload: payload,
        qos: 0,
      },
      (err) => {
        if (err) {
          console.log(`❌ Failed to send force_update to ${moduleId}: ${err.message}`);
          resolve(false);
        } else {
          console.log(`✅ Successfully sent force_update command to ${moduleId}`);
          resolve(true);
        }
      }
    );
  });
};

/**
 * Send force update command to all online modules
 * @param {Object} aedes - Aedes MQTT broker instance
 * @param {Object} moduleRegistry - ModuleRegistry instance
 * @returns {Promise<number>} Number of commands sent successfully
 */
const sendForceUpdateToAllOnline = async (aedes, moduleRegistry) => {
  const allStatuses = moduleRegistry.getAllModuleStatuses();
  console.log(`📊 Current module statuses:`, Object.keys(allStatuses));
  
  const onlineModules = Object.keys(allStatuses).filter(
    (moduleId) => allStatuses[moduleId]?.status === "online"
  );

  console.log(`📊 Online modules:`, onlineModules);

  if (onlineModules.length === 0) {
    console.log("⚠️ No online modules to send force_update to");
    return 0;
  }

  console.log(`🔄 Requesting fresh data from ${onlineModules.length} online module(s)...`);

  // Send commands to all online modules
  const results = await Promise.all(
    onlineModules.map((moduleId) => sendForceUpdate(aedes, moduleId))
  );

  const successCount = results.filter((success) => success).length;
  console.log(`✅ Sent force_update to ${successCount}/${onlineModules.length} module(s)`);

  return successCount;
};

module.exports = {
  sendForceUpdate,
  sendForceUpdateToAllOnline,
};

