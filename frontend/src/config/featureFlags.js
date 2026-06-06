/**
 * Feature flags — toggle UI without deleting implementation.
 *
 * @see ../utils/moduleGating.js
 * @see ../components/StatusIcons.jsx
 * @see ../hooks/useModuleStatus.js — socket disconnect disables all modules
 */

export const FEATURE_FLAGS = {
  /** Per-module WiFi/status icons (antennas 1–6) in header. */
  moduleStatusIcons: false,
};
