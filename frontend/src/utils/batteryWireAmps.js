import { BATTERY_WIRE_POWER_LABELS } from "../config/batterySystemNodes";

/**
 * Wire current (A) / power (W) labels — derive from node metrics when backend omits wireAmps.
 */

/**
 * @param {number|null|undefined} power
 * @param {number|null|undefined} voltage
 */
function ampsFromPowerVoltage(power, voltage) {
  if (power == null || voltage == null || voltage <= 0) return 0;
  return Math.round((Number(power) / Number(voltage)) * 10) / 10;
}

/**
 * @param {Object} nodes
 * @returns {Record<string, number>}
 */
export function deriveWireAmpsFromNodes(nodes = {}) {
  return {
    "solar1-mppt1": Number(nodes.solarPanelGroup1?.power) || 0,
    "solar2-mppt2": Number(nodes.solarPanelGroup2?.power) || 0,
    "alternator-dcdc": Number(nodes.alternator?.current) || 0,
    "dcdc-battery": Number(nodes.dcDcBooster?.current) || 0,
    "mppt1-battery": ampsFromPowerVoltage(
      nodes.solarController1?.power,
      nodes.solarController1?.voltage
    ),
    "mppt2-battery": ampsFromPowerVoltage(
      nodes.solarController2?.power,
      nodes.solarController2?.voltage
    ),
    "loads-battery": ampsFromPowerVoltage(
      nodes.dcLoads?.power,
      nodes.dcLoads?.voltage
    ),
    "ac-battery": Number(nodes.charger230v?.batteryCurrent) || 0,
  };
}

/**
 * @param {number|null|undefined} amps
 */
export function formatWireAmps(amps) {
  if (amps == null || Number.isNaN(Number(amps))) return "—A";
  return `${Number(amps).toFixed(1)}A`;
}

/**
 * @param {number|null|undefined} watts
 */
export function formatWirePower(watts) {
  if (watts == null || Number.isNaN(Number(watts))) return "—W";
  return `${Math.round(Number(watts))}W`;
}

/**
 * Combined PV power from both solar panel groups (W).
 * @param {Record<string, number>} wireAmps
 */
export function getTotalSolarPower(wireAmps = {}) {
  const solar1 = Number(wireAmps["solar1-mppt1"]) || 0;
  const solar2 = Number(wireAmps["solar2-mppt2"]) || 0;
  return Math.round(solar1 + solar2);
}

/**
 * @param {string} wireId
 * @param {number|null|undefined} value
 */
export function formatWireLabel(wireId, value) {
  if (BATTERY_WIRE_POWER_LABELS.has(wireId)) {
    return formatWirePower(value);
  }
  return formatWireAmps(value);
}

/**
 * @param {string} wireId
 * @param {number|null|undefined} value
 */
export function hasWireLabelValue(wireId, value) {
  if (BATTERY_WIRE_POWER_LABELS.has(wireId)) {
    return Number(value) > 0;
  }
  return hasWireCurrent(value);
}

const WIRE_MIN_VISIBLE_AMPS = 0.05;

/** Wires that deliver current into the battery. */
export const BATTERY_CHARGE_WIRES = [
  "dcdc-battery",
  "mppt1-battery",
  "mppt2-battery",
  "ac-battery",
];

/** Wires that draw current from the battery. */
export const BATTERY_DISCHARGE_WIRES = ["loads-battery"];

const DEFAULT_BATTERY_VOLTAGE = 12.6;

/**
 * @param {number|null|undefined} amps
 */
export function hasWireCurrent(amps) {
  return Number(amps) > WIRE_MIN_VISIBLE_AMPS;
}

/**
 * Net battery flow from wire currents (charge in minus discharge out).
 * @param {Record<string, number>} wireAmps
 * @param {number} [batteryVoltage]
 * @returns {{ direction: 'charge' | 'discharge' | 'idle', amps: number, watts: number, netAmps: number, voltage: number }}
 */
export function computeBatteryFlow(wireAmps = {}, batteryVoltage = DEFAULT_BATTERY_VOLTAGE) {
  const chargeIn = BATTERY_CHARGE_WIRES.reduce(
    (sum, id) => sum + (Number(wireAmps[id]) || 0),
    0
  );
  const dischargeOut = BATTERY_DISCHARGE_WIRES.reduce(
    (sum, id) => sum + (Number(wireAmps[id]) || 0),
    0
  );
  const netAmps = Math.round((chargeIn - dischargeOut) * 10) / 10;
  const absAmps = Math.round(Math.abs(netAmps) * 10) / 10;
  const watts = Math.round(absAmps * batteryVoltage);

  let direction = "idle";
  if (netAmps > WIRE_MIN_VISIBLE_AMPS) direction = "charge";
  else if (netAmps < -WIRE_MIN_VISIBLE_AMPS) direction = "discharge";

  return { direction, amps: absAmps, watts, netAmps, voltage: batteryVoltage };
}
