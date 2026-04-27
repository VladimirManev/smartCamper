/** UI copy for appearance (settings + tablet chip) — keep in sync */
export const APPEARANCE_MODE_OPTIONS = [
  { value: "day", label: "Daily mode" },
  { value: "night", label: "Night mode" },
  { value: "automatic", label: "Auto mode" },
];

export function getAppearanceModeLabel(mode) {
  const row = APPEARANCE_MODE_OPTIONS.find((o) => o.value === mode);
  return row?.label ?? mode;
}
