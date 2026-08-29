// SmartCamper Backend Server
// Main file for Express server

const express = require("express");
const http = require("http");
const { Server } = require("socket.io");

// Import middleware
const corsMiddleware = require("./middleware/cors");
const loggerMiddleware = require("./middleware/logger");
const { staticMiddleware, fallbackMiddleware } = require("./middleware/static");

// Import routes
const mainRoutes = require("./routes/main");
const historyRoutes = require("./routes/history");
const notFoundRoutes = require("./routes/404");

// Import MQTT broker
const setupMQTTBroker = require("./mqtt/broker");

// Import Socket.io handler
const setupSocketIO = require("./socket/socketHandler");

// Create Express application
const app = express();

// Create HTTP server (needed for Socket.io)
const server = http.createServer(app);

// Create Socket.io server
const io = new Server(server, {
  cors: {
    origin: "*", // Allow all origins (for development)
    methods: ["GET", "POST"],
  },
});

// Middleware for parsing JSON data
app.use(express.json());

// Custom middleware
app.use(corsMiddleware);
app.use(loggerMiddleware);

// API Routes - must be before static middleware
app.use("/", mainRoutes);
app.use("/api/history", historyRoutes);

// Static files middleware - serves React build files
// Must be before fallback middleware
app.use(staticMiddleware);

// Fallback middleware - redirects all routes to index.html (for React Router)
// Must be before 404 handler
app.use(fallbackMiddleware);

// 404 handler - must be last!
app.use(notFoundRoutes);

// Initialize MQTT broker
const aedes = setupMQTTBroker();

// Initialize Socket.io with MQTT Bridge
setupSocketIO(io, aedes);

// Start HTTP + WebSocket server on port 3000
const PORT = 3000;
server.listen(PORT, () => {
  console.log(`🚀 SmartCamper Backend running on port ${PORT}`);
  console.log(`📡 HTTP: http://localhost:${PORT}`);
  console.log(`💚 Health: http://localhost:${PORT}/health`);
  console.log(`📜 History: http://localhost:${PORT}/api/history/readings?metric=soc&hours=24`);
  console.log(`🔌 WebSocket: ws://localhost:${PORT}`);
});
