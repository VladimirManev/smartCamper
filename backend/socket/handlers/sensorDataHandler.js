/**
 * Sensor Data Handler
 * Handles sensor data messages from ESP32 modules
 */

const sensorDataHandler = (io, topic, message) => {
  // Topic format: smartcamper/sensors/{sensor-type}
  if (!topic.startsWith("smartcamper/sensors/")) {
    return false; // Not a sensor topic
  }
  
  const topicParts = topic.split("/");
  const sensorType = topicParts[2]; // smartcamper/sensors/temperature
  
  // Handle different sensor types
  switch (sensorType) {
    case "temperature":
      return handleTemperature(io, message);
    
    case "humidity":
      return handleHumidity(io, message);
    
    case "gray-water":
      return handleGrayWater(io, topicParts, message);
    
    case "led-controller":
      return handleLEDController(io, topicParts, message);
    
    default:
      return false; // Unknown sensor type
  }
};

/**
 * Handle temperature sensor data
 */
function handleTemperature(io, message) {
  const value = parseFloat(message);
  
  if (isNaN(value)) {
    return false;
  }
  
  // Emit sensor update
  io.emit("sensorUpdate", {
    temperature: value,
    timestamp: new Date().toISOString(),
  });
  
  return true;
}

/**
 * Handle humidity sensor data
 */
function handleHumidity(io, message) {
  const value = parseInt(message);
  
  if (isNaN(value)) {
    return false;
  }
  
  // Emit sensor update
  io.emit("sensorUpdate", {
    humidity: value,
    timestamp: new Date().toISOString(),
  });
  
  return true;
}

/**
 * Handle gray water level sensor data
 */
function handleGrayWater(io, topicParts, message) {
  // Format: smartcamper/sensors/gray-water/level
  if (topicParts.length >= 4 && topicParts[3] === "level") {
    const value = parseFloat(message);
    
    if (!isNaN(value) && value >= 0 && value <= 100) {
      io.emit("sensorUpdate", {
        grayWaterLevel: value,
        timestamp: new Date().toISOString(),
      });
      
      return true;
    } else {
      console.log(`⚠️ Invalid gray water level value: ${message} (must be 0-100)`);
      return true; // Handled, but invalid
    }
  }
  
  return false;
}

/**
 * Handle LED controller status data
 */
function handleLEDController(io, topicParts, message) {
  // Format: smartcamper/sensors/led-controller/status (JSON)
  if (topicParts.length >= 4 && topicParts[3] === "status") {
    try {
      const statusData = JSON.parse(message);
      
      // Send full status to frontend
      io.emit("ledStatusUpdate", {
        type: "full",
        data: statusData,
        timestamp: new Date().toISOString(),
      });
      
      return true;
    } catch (error) {
      console.log(`❌ Failed to parse LED status JSON: ${error.message}`);
      return true; // Handled, but error
    }
  }
  
  // Legacy format support (for backward compatibility)
  // Format: smartcamper/sensors/led-controller/strip/{index}/{type}
  if (topicParts.length >= 6 && topicParts[3] === "strip") {
    const stripIndex = parseInt(topicParts[4]);
    const dataType = topicParts[5]; // state or brightness
    
    if (!isNaN(stripIndex) && (dataType === "state" || dataType === "brightness")) {
      io.emit("ledStatusUpdate", {
        type: "strip",
        index: stripIndex,
        dataType: dataType,
        value: dataType === "brightness" ? parseInt(message) : message,
        timestamp: new Date().toISOString(),
      });
      
      return true;
    }
  }
  
  // Legacy relay format
  if (topicParts.length >= 5 && topicParts[3] === "relay") {
    const dataType = topicParts[4]; // state
    
    if (dataType === "state") {
      io.emit("ledStatusUpdate", {
        type: "relay",
        dataType: "state",
        value: message,
        timestamp: new Date().toISOString(),
      });
      
      return true;
    }
  }
  
  return false;
}

module.exports = sensorDataHandler;

