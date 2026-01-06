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
    case "indoor-temperature":
      return handleIndoorTemperature(io, message);
    
    case "indoor-humidity":
      return handleIndoorHumidity(io, message);
    
    case "outdoor-temperature":
      return handleOutdoorTemperature(io, message);
    
    case "gray-water":
      return handleGrayWater(io, topicParts, message);
    
    case "gray-water-temperature":
      return handleGrayWaterTemperature(io, message);
    
    case "led-controller":
      return handleLEDController(io, topicParts, message);
    
    case "module-2":
      // Module-2 is the LED controller module (renamed from led-controller)
      return handleLEDController(io, topicParts, message);
    
    case "module-3":
      // Module-3 is the floor heating module
      return handleFloorHeating(io, topicParts, message);
    
    case "errors":
      // Error topics: smartcamper/errors/{module-id}/{component-type}/{component-id}
      return handleErrorTopic(io, topicParts, message);
    
    // Legacy temperature-only topics removed - temperature is now included in full status
    // case "floor-heating-circle-0-temp": etc. - no longer used
    
    default:
      return false; // Unknown sensor type
  }
};

/**
 * Handle indoor temperature sensor data (DHT22)
 */
function handleIndoorTemperature(io, message) {
  const value = parseFloat(message);
  
  if (isNaN(value)) {
    return false;
  }
  
  // Emit sensor update
  io.emit("sensorUpdate", {
    indoorTemperature: value,
    timestamp: new Date().toISOString(),
  });
  
  return true;
}

/**
 * Handle indoor humidity sensor data (DHT22)
 */
function handleIndoorHumidity(io, message) {
  const value = parseInt(message);
  
  if (isNaN(value)) {
    return false;
  }
  
  // Emit sensor update
  io.emit("sensorUpdate", {
    indoorHumidity: value,
    timestamp: new Date().toISOString(),
  });
  
  return true;
}

/**
 * Handle outdoor temperature sensor data (DS18B20)
 */
function handleOutdoorTemperature(io, message) {
  const value = parseFloat(message);
  
  if (isNaN(value)) {
    return false;
  }
  
  // Emit sensor update
  io.emit("sensorUpdate", {
    outdoorTemperature: value,
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
 * Handle gray water temperature sensor data (DS18B20)
 */
function handleGrayWaterTemperature(io, message) {
  const value = parseFloat(message);
  
  if (isNaN(value)) {
    return false;
  }
  
  // Emit sensor update
  io.emit("sensorUpdate", {
    grayWaterTemperature: value,
    timestamp: new Date().toISOString(),
  });
  
  return true;
}

/**
 * Handle LED controller status data
 * Supports both: smartcamper/sensors/led-controller/status and smartcamper/sensors/module-2/status
 */
function handleLEDController(io, topicParts, message) {
  // Format: smartcamper/sensors/{module-id}/status (JSON)
  // Supports both "led-controller" and "module-2"
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

/**
 * Handle floor heating status data
 * Format: smartcamper/sensors/module-3/status (JSON)
 */
function handleFloorHeating(io, topicParts, message) {
  // Format: smartcamper/sensors/module-3/status (JSON)
  if (topicParts.length >= 4 && topicParts[3] === "status") {
    try {
      const statusData = JSON.parse(message);
      
      // Forward the status data as-is from module (new format)
      // Module publishes: {type: "circle", index: 0, mode: "TEMP_CONTROL", relay: "ON", temperature: 22, error: false}
      // or: {type: "full", data: {circles: {...}}}
      io.emit("floorHeatingStatusUpdate", {
        ...statusData, // Forward all fields from module
        timestamp: new Date().toISOString(),
      });
      
      return true;
    } catch (error) {
      console.log(`❌ Failed to parse floor heating status JSON: ${error.message}`);
      return true; // Handled, but error
    }
  }
  
  return false;
}

// Legacy handleFloorHeatingTemperature function removed
// Temperature is now included in full status published by module-3

/**
 * Handle error topics
 * Format: smartcamper/errors/{module-id}/{component-type}/{component-id}
 */
function handleErrorTopic(io, topicParts, message) {
  // Format: smartcamper/errors/module-3/circle/0
  if (topicParts.length >= 5) {
    try {
      const errorData = JSON.parse(message);
      
      // Extract module ID and component info
      const moduleId = topicParts[2];  // module-3
      const componentType = topicParts[3];  // circle
      const componentId = topicParts[4];  // 0, 1, 2, 3
      
      // For module-3, emit error as floor heating status update
      if (moduleId === "module-3" && componentType === "circle") {
        io.emit("floorHeatingStatusUpdate", {
          type: "error",
          index: parseInt(componentId),
          error: errorData,
          timestamp: new Date().toISOString(),
        });
      }
      
      return true;
    } catch (error) {
      console.log(`❌ Failed to parse error JSON: ${error.message}`);
      return true; // Handled, but error
    }
  }
  
  return false;
}

module.exports = sensorDataHandler;

