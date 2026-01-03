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

    // Create socket connection
    const socketInstance = io(socketUrl);
    setSocket(socketInstance);

    // Connection event handlers
    socketInstance.on("connect", () => {
      setConnected(true);
      setError(null);
    });

    socketInstance.on("disconnect", () => {
      setConnected(false);
    });

    socketInstance.on("connect_error", (err) => {
      console.error("❌ Connection error:", err);
      setError(err.message);
      setConnected(false);
    });

    // Cleanup on unmount
    return () => {
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

