/** Module-5 appliance relay indices */
export const APPLIANCE_INDEX = {
  audio: 0,
  pump: 1,
  fridge: 2,
  wcFan: 3,
  boiler: 4,
  inverter: 5,
  /** Tablet display backlight — relay 6 on module-5, not shown in appliance cards */
  displayBacklight: 6,
};

export function isInverterOn(appliances) {
  return appliances[APPLIANCE_INDEX.inverter]?.state === "ON";
}

export function isBoilerOn(appliances) {
  return appliances[APPLIANCE_INDEX.boiler]?.state === "ON";
}

/**
 * 230 V for the boiler: inverter relay ON, or live Victron AC charger (shore power).
 * @param {Object} appliances
 * @param {boolean} isAcChargerLive
 */
export function hasBoilerAcSupply(appliances, isAcChargerLive) {
  return isInverterOn(appliances) || Boolean(isAcChargerLive);
}

/**
 * Boiler usable when module-5 is online and AC is available (inverter or shore).
 * If already ON, always allow control so it can be turned off.
 */
export function isBoilerControlEnabled(
  appliances,
  isModuleOnline,
  isAcChargerLive = false
) {
  if (!isModuleOnline) return false;
  if (isBoilerOn(appliances)) return true;
  return hasBoilerAcSupply(appliances, isAcChargerLive);
}

/** Force boiler OFF when neither inverter nor shore AC is available. */
export function shouldAutoOffBoiler(appliances, isAcChargerLive) {
  return isBoilerOn(appliances) && !hasBoilerAcSupply(appliances, isAcChargerLive);
}

/**
 * Commands to send for an appliance toggle (boiler/inverter dependency rules).
 * @returns {Array<{ type: string, index: number, action: string }>}
 */
export function getApplianceToggleCommands(
  appliances,
  index,
  isAcChargerLive = false
) {
  if (index === APPLIANCE_INDEX.boiler) {
    const turningOn = !isBoilerOn(appliances);
    if (turningOn && !hasBoilerAcSupply(appliances, isAcChargerLive)) {
      return [];
    }
  }

  const commands = [];

  // Turning inverter OFF: turn boiler OFF only if shore AC is also absent
  if (
    index === APPLIANCE_INDEX.inverter &&
    appliances[APPLIANCE_INDEX.inverter]?.state === "ON" &&
    isBoilerOn(appliances) &&
    !isAcChargerLive
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
