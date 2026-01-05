/**
 * Socket.io Handler + MQTT Bridge
 * Coordinates WebSocket communication and MQTT message handling
 * Uses separate handlers for different concerns
 */

const ModuleRegistry = require("../src/ModuleRegistry");
const heartbeatHandler = require("./handlers/heartbeatHandler");
const sensorDataHandler = require("./handlers/sensorDataHandler");
const ledCommandHandler = require("./handlers/ledCommandHandler");
const floorHeatingCommandHandler = require("./handlers/floorHeatingCommandHandler");
const { sendForceUpdateToAllOnline } = require("./handlers/moduleCommandHandler");

const setupSocketIO = (io, aedes) => {
  // Initialize Module Registry for heartbeat tracking
  const moduleRegistry = new ModuleRegistry();
  
  // Callback when module status changes
  const onModuleStatusChange = (allStatuses) => {
    // Broadcast module status update to all connected clients
    io.emit("moduleStatusUpdate", {
      modules: allStatuses,
      timestamp: new Date().toISOString(),
    });
  };
  
  // Initialize registry with status change callback
  moduleRegistry.initialize(onModuleStatusChange);
  
  // MQTT ↔ WebSocket Bridge - listen to Aedes broker directly
  aedes.on("publish", (packet, client) => {
    const topic = packet.topic;
    const message = packet.payload.toString();

    // Log MQTT message (only if DEBUG_MQTT is set)
    if (process.env.DEBUG_MQTT) {
      console.log(`📨 MQTT: ${topic} = ${message}`);
    }

    // Try heartbeat handler first (most specific)
    if (heartbeatHandler(moduleRegistry, io, topic, message)) {
      return; // Handled by heartbeat handler
    }

    // Try sensor data handler
    if (sensorDataHandler(io, topic, message)) {
      return; // Handled by sensor data handler
    }

    // Unknown topic - log for debugging
    if (process.env.DEBUG_MQTT) {
      console.log(`⚠️ Unhandled MQTT topic: ${topic}`);
    }
  });

  // WebSocket connection events
  io.on("connection", (socket) => {
    console.log("✅ Frontend connected via WebSocket");

    // Send current module statuses to newly connected client
    const allStatuses = moduleRegistry.getAllModuleStatuses();
    socket.emit("moduleStatusUpdate", {
      modules: allStatuses,
      timestamp: new Date().toISOString(),
    });

    // Request fresh data from all online modules when frontend connects
    // Add small delay to ensure module registry is up to date
    setTimeout(() => {
      sendForceUpdateToAllOnline(aedes, moduleRegistry).catch((err) => {
        console.log(`❌ Error requesting fresh data: ${err.message}`);
      });
    }, 500); // 500ms delay to ensure modules are registered

    // Handle LED commands from frontend
    socket.on("ledCommand", (data) => {
      ledCommandHandler(socket, aedes, data);
    });

    // Handle floor heating commands from frontend
    socket.on("floorHeatingCommand", (data) => {
      floorHeatingCommandHandler(socket, aedes, data);
    });

    // When frontend disconnects
    socket.on("disconnect", () => {
      console.log("❌ Frontend disconnected");
    });
  });

  // Cleanup on server shutdown
  process.on("SIGINT", () => {
    console.log("🛑 Shutting down Module Registry...");
    moduleRegistry.stop();
  });

  process.on("SIGTERM", () => {
    console.log("🛑 Shutting down Module Registry...");
    moduleRegistry.stop();
  });

  return {
    moduleRegistry, // Expose registry for potential API access
  };
};

module.exports = setupSocketIO;
