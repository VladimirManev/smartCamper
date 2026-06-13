/** Module-5 appliance relay indices */
export const APPLIANCE_INDEX = {
  audio: 0,
  pump: 1,
  fridge: 2,
  wcFan: 3,
  boiler: 4,
  inverter: 5,
};

export function isInverterOn(appliances) {
  return appliances[APPLIANCE_INDEX.inverter]?.state === "ON";
}

/** Boiler is usable only when module-5 is online and inverter is ON. */
export function isBoilerControlEnabled(appliances, isModuleOnline) {
  return isModuleOnline && isInverterOn(appliances);
}

/**
 * Commands to send for an appliance toggle (boiler/inverter dependency rules).
 * @returns {Array<{ type: string, index: number, action: string }>}
 */
export function getApplianceToggleCommands(appliances, index) {
  if (index === APPLIANCE_INDEX.boiler && !isInverterOn(appliances)) {
    return [];
  }

  const commands = [];

  if (
    index === APPLIANCE_INDEX.inverter &&
    appliances[APPLIANCE_INDEX.inverter]?.state === "ON" &&
    appliances[APPLIANCE_INDEX.boiler]?.state === "ON"
  ) {
    commands.push({
      type: "relay",
      index: APPLIANCE_INDEX.boiler,
      action: "toggle",
    });
  }

  commands.push({ type: "relay", index, action: "toggle" });
  return commands;
}
