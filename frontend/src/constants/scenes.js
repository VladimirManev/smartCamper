/**
 * Scene presets — labels and icons for the Scenes modal.
 */

export const SCENES = [
  { id: "normal", label: "Normal", icon: "house" },
  { id: "drive", label: "Drive", icon: "car" },
  { id: "film", label: "Cinema", icon: "film" },
  { id: "sleep", label: "Sleep", icon: "bed" },
  { id: "cooking", label: "Cooking", icon: "utensils" },
  { id: "all-off", label: "All Off", icon: "power-off" },
];

/** Scene is disabled when any listed module is offline. */
export const SCENE_REQUIRED_MODULES = {
  normal: ["module-2", "module-5"],
  drive: ["module-2", "module-3", "module-5"],
  film: ["module-2", "module-5"],
  sleep: ["module-2", "module-5"],
  cooking: ["module-2", "module-5"],
  "all-off": ["module-2", "module-3", "module-5"],
};
