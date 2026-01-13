/**
 * Heartbeat Handler
 * Handles heartbeat messages from ESP32 modules
 */

const heartbeatHandler = (moduleRegistry, io, topic, message) => {
  // Topic format: smartcamper/heartbeat/{module-id}
  const topicParts = topic.split("/");

  if (
    topicParts.length !== 3 ||
    topicParts[0] !== "smartcamper" ||
    topicParts[1] !== "heartbeat"
  ) {
    return false; // Not a heartbeat topic
  }

  const moduleId = topicParts[2];

  try {
    // Parse heartbeat payload (JSON)
    const heartbeatData = JSON.parse(message);

    // Validate heartbeat data
    if (!heartbeatData.moduleId || heartbeatData.moduleId !== moduleId) {
      console.log(
        `⚠️ Heartbeat moduleId mismatch: expected ${moduleId}, got ${heartbeatData.moduleId}`
      );
      return true; // Handled, but invalid
    }

    // Check for reset reason (only present in first heartbeat after boot)
    if (heartbeatData.resetReason) {
      const timestamp = new Date().toISOString();
      console.log(`\n🔍 === MODULE RESTART DETECTED ===`);
      console.log(`Module: ${moduleId}`);
      console.log(`Reset Reason: ${heartbeatData.resetReason}`);
      console.log(`Timestamp: ${timestamp}`);
      console.log(`Uptime: ${heartbeatData.uptime || 0} seconds`);
      if (heartbeatData.wifiRSSI && heartbeatData.wifiRSSI !== -999) {
        console.log(`WiFi RSSI: ${heartbeatData.wifiRSSI} dBm`);
      }
      console.log(`===================================\n`);
    }

    // Process heartbeat in registry
    moduleRegistry.processHeartbeat(moduleId, heartbeatData);

    if (process.env.DEBUG_MQTT) {
      console.log(`💓 Heartbeat from ${moduleId}:`, heartbeatData);
    }

    return true; // Handled
  } catch (error) {
    console.log(
      `❌ Failed to parse heartbeat from ${moduleId}: ${error.message}`
    );
    console.log(`   Raw message: ${message}`);
    return true; // Handled, but error
  }
};

module.exports = heartbeatHandler;
