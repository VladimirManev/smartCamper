/**
 * Victron Instant Readout `deviceState` → short UI label (charger phase).
 * @see Victron BLE Instant Readout / VE.Direct DeviceState
 */

const DEVICE_STATE_LABELS = {
  0: "OFF",
  1: "LOW",
  2: "FAULT",
  3: "BULK",
  4: "ABS",
  5: "FLOAT",
  6: "STOR",
  7: "EQ",
};

/**
 * @param {number|null|undefined} deviceState
 * @returns {string|null}
 */
export function formatVictronDeviceState(deviceState) {
  if (deviceState == null || Number.isNaN(Number(deviceState))) {
    return null;
  }
  const code = Number(deviceState);
  return DEVICE_STATE_LABELS[code] ?? String(code);
}

/** Nodes that show charger phase on the energy diagram badge. */
export const BATTERY_CHARGER_PHASE_NODES = [
  "charger230v",
  "dcDcBooster",
  "solarController1",
  "solarController2",
];
