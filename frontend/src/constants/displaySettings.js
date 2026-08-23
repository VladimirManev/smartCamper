/** Display backlight auto-off — tablet landscape only */

export const DISPLAY_AUTO_OFF_STORAGE_KEY = "smartcamper_display_auto_off";

export const DISPLAY_AUTO_OFF_NEVER = "never";

export const DISPLAY_AUTO_OFF_OPTIONS = [
  { value: DISPLAY_AUTO_OFF_NEVER, label: "Never" },
  { value: "15", label: "15 sec" },
  { value: "120", label: "2 min" },
  { value: "300", label: "5 min" },
  { value: "600", label: "10 min" },
  { value: "1800", label: "30 min" },
  { value: "3600", label: "1 hour" },
];

export function getDisplayAutoOffSeconds(value) {
  if (!value || value === DISPLAY_AUTO_OFF_NEVER) {
    return null;
  }
  const seconds = Number(value);
  return Number.isFinite(seconds) && seconds > 0 ? seconds : null;
}

export function loadDisplayAutoOffSetting() {
  if (typeof window === "undefined") {
    return DISPLAY_AUTO_OFF_NEVER;
  }
  const stored = window.localStorage.getItem(DISPLAY_AUTO_OFF_STORAGE_KEY);
  if (DISPLAY_AUTO_OFF_OPTIONS.some((option) => option.value === stored)) {
    return stored;
  }
  return DISPLAY_AUTO_OFF_NEVER;
}

export function saveDisplayAutoOffSetting(value) {
  if (typeof window === "undefined") {
    return;
  }
  window.localStorage.setItem(DISPLAY_AUTO_OFF_STORAGE_KEY, value);
}
