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
    const isDevelopment = import.meta.env.DEV;
    const mockPort = import.meta.env.VITE_MOCK_SOCKET_PORT || "3100";
    const envMock = import.meta.env.VITE_MOCK_BACKEND;
    const usePiBackend = import.meta.env.VITE_USE_PI_BACKEND === "true";

    const isLocalhost =
      window.location.hostname === "localhost" ||
      window.location.hostname === "127.0.0.1";

    // Dev on this machine: Pi at 192.168.4.1 is usually unreachable from localhost:5173.
    // Use mock on localhost unless explicitly pointed at the Pi.
    const useMockBackend =
      envMock === "true" ||
      (isDevelopment && isLocalhost && envMock !== "false" && !usePiBackend);

    const socketUrl = useMockBackend
      ? `http://localhost:${mockPort}`
      : isDevelopment
        ? "http://192.168.4.1:3000"
        : window.location.origin;

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

