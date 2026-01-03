/**
 * useSocket Hook
 * Manages WebSocket connection to backend
 * Provides socket instance and connection status
 */

import { useState, useEffect } from "react";
import io from "socket.io-client";

/**
 * Custom hook for Socket.io connection
 * @returns {Object} { socket, connected, error }
 */
export const useSocket = () => {
  const [socket, setSocket] = useState(null);
  const [connected, setConnected] = useState(false);
  const [error, setError] = useState(null);

  useEffect(() => {
    // Determine socket URL based on environment
    const isDevelopment = import.meta.env.DEV;
    const socketUrl = isDevelopment
      ? "http://192.168.4.1:3000" // Raspberry Pi IP for development
      : window.location.origin; // Production - same host

    console.log("🔌 Creating socket connection to:", socketUrl);

    // Create socket connection
    const socketInstance = io(socketUrl);
    setSocket(socketInstance);

    // Connection event handlers
    socketInstance.on("connect", () => {
      console.log("✅ Connected to backend");
      setConnected(true);
      setError(null);
    });

    socketInstance.on("disconnect", () => {
      console.log("❌ Disconnected from backend");
      setConnected(false);
    });

    socketInstance.on("connect_error", (err) => {
      console.error("❌ Connection error:", err);
      setError(err.message);
      setConnected(false);
    });

    // Debug: Log all events for troubleshooting
    socketInstance.onAny((eventName, ...args) => {
      if (eventName !== "sensorData") { // Skip frequent sensor data events
        console.log(`📡 Socket event: ${eventName}`, args);
      }
    });

    // Cleanup on unmount
    return () => {
      console.log("🔌 Disconnecting socket...");
      socketInstance.disconnect();
      setSocket(null);
    };
  }, []);

  return {
    socket,
    connected,
    error,
  };
};

