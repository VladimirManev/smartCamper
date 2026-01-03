/**
 * useSocket Hook
 * Manages WebSocket connection to backend
 * Provides socket instance and connection status
 */

import { useState, useEffect, useRef } from "react";
import io from "socket.io-client";

/**
 * Custom hook for Socket.io connection
 * @returns {Object} { socket, connected, error }
 */
export const useSocket = () => {
  const [connected, setConnected] = useState(false);
  const [error, setError] = useState(null);
  const socketRef = useRef(null);

  useEffect(() => {
    // Determine socket URL based on environment
    const isDevelopment = import.meta.env.DEV;
    const socketUrl = isDevelopment
      ? "http://192.168.4.1:3000" // Raspberry Pi IP for development
      : window.location.origin; // Production - same host

    // Create socket connection
    const socket = io(socketUrl);
    socketRef.current = socket;

    // Connection event handlers
    socket.on("connect", () => {
      console.log("✅ Connected to backend");
      setConnected(true);
      setError(null);
    });

    socket.on("disconnect", () => {
      console.log("❌ Disconnected from backend");
      setConnected(false);
    });

    socket.on("connect_error", (err) => {
      console.error("❌ Connection error:", err);
      setError(err.message);
      setConnected(false);
    });

    // Cleanup on unmount
    return () => {
      socket.disconnect();
      socketRef.current = null;
    };
  }, []);

  return {
    socket: socketRef.current,
    connected,
    error,
  };
};

