/**
 * Heartbeat Handler
 * Handles heartbeat messages from ESP32 modules
 */

const heartbeatHandler = (moduleRegistry, io, topic, message) => {
  // Topic format: smartcamper/heartbeat/{module-id}
  const topicParts = topic.split("/");
  
  // Debug: Log all heartbeat-related topics
  if (topicParts[1] === "heartbeat") {
    console.log(`🔍 Heartbeat topic received: ${topic}, parts:`, topicParts);
  }
  
  if (topicParts.length !== 3 || topicParts[0] !== "smartcamper" || topicParts[1] !== "heartbeat") {
    return false; // Not a heartbeat topic
  }
  
  const moduleId = topicParts[2];
  
  try {
    // Parse heartbeat payload (JSON)
    const heartbeatData = JSON.parse(message);
    
    console.log(`💓 Processing heartbeat from ${moduleId}:`, heartbeatData);
    
    // Validate heartbeat data
    if (!heartbeatData.moduleId || heartbeatData.moduleId !== moduleId) {
      console.log(`⚠️ Heartbeat moduleId mismatch: expected ${moduleId}, got ${heartbeatData.moduleId}`);
      return true; // Handled, but invalid
    }
    
    // Process heartbeat in registry
    moduleRegistry.processHeartbeat(moduleId, heartbeatData);
    
    console.log(`✅ Heartbeat processed for ${moduleId}`);
    
    return true; // Handled
  } catch (error) {
    console.log(`❌ Failed to parse heartbeat from ${moduleId}: ${error.message}`);
    console.log(`   Raw message: ${message}`);
    return true; // Handled, but error
  }
};

module.exports = heartbeatHandler;

