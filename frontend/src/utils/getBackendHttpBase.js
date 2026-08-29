/**
 * Resolve HTTP base URL for backend REST calls (same rules as useSocket).
 * @returns {string}
 */
export function getBackendHttpBase() {
  const isDevelopment = import.meta.env.DEV;
  const mockPort = import.meta.env.VITE_MOCK_SOCKET_PORT || "3100";
  const envMock = import.meta.env.VITE_MOCK_BACKEND;
  const usePiBackend = import.meta.env.VITE_USE_PI_BACKEND === "true";

  const isLocalhost =
    window.location.hostname === "localhost" ||
    window.location.hostname === "127.0.0.1";

  const useMockBackend =
    envMock === "true" ||
    (isDevelopment && isLocalhost && envMock !== "false" && !usePiBackend);

  if (useMockBackend) {
    return `http://localhost:${mockPort}`;
  }

  if (isDevelopment) {
    return "http://192.168.4.1:3000";
  }

  return window.location.origin;
}
