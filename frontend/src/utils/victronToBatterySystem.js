/**
 * Map module-6 Victron MQTT/WebSocket payload to battery diagram nodes and wires.
 */

import { BATTERY_NODE_IDS } from "../config/batterySystemNodes";

export const VICTRON_STALE_MS = 6000;

const WIRE_MIN_VISIBLE_AMPS = 0.05;

const EMPTY_NODES = Object.fromEntries(BATTERY_NODE_IDS.map((id) => [id, null]));

/**
 * @param {number|null|undefined} publishedAt
 * @param {number|null|undefined} updatedAt
 */
export function isVictronDeviceStale(publishedAt, updatedAt) {
  if (publishedAt == null || updatedAt == null) return true;
  return publishedAt - updatedAt > VICTRON_STALE_MS;
}

/**
 * @param {Object|null|undefined} device
 * @param {number|null|undefined} publishedAt
 */
export function getFreshVictronDevice(device, publishedAt) {
  if (!device || typeof device !== "object") return null;
  if (isVictronDeviceStale(publishedAt, device.updatedAt)) return null;
  return device;
}

function round2(value) {
  return Math.round(Number(value) * 100) / 100;
}

function estimatePanelAmps(mppt) {
  if (!mppt || !mppt.pvPower || mppt.pvPower <= 0) return 0;
  const voltage = mppt.batteryVoltage ?? 12.6;
  if (voltage <= 0) return 0;
  return Math.round((mppt.pvPower / voltage) * 10) / 10;
}

/**
 * @param {Object|null|undefined} shunt
 */
export function computeFlowFromSmartShunt(shunt) {
  if (!shunt || shunt.current == null) {
    return { direction: "idle", amps: 0, watts: 0, netAmps: 0 };
  }

  const netAmps = round2(shunt.current);
  const voltage = Number(shunt.voltage) || 12.6;
  const absAmps = Math.round(Math.abs(netAmps) * 10) / 10;
  const watts = Math.round(absAmps * voltage);

  let direction = "idle";
  if (netAmps > WIRE_MIN_VISIBLE_AMPS) direction = "charge";
  else if (netAmps < -WIRE_MIN_VISIBLE_AMPS) direction = "discharge";

  return { direction, amps: absAmps, watts, netAmps };
}

/**
 * @param {Object|null|undefined} victronData
 * @returns {{ nodes: Object, wireAmps: Object, batteryLevel: number|null, batteryFlow: Object }}
 */
export function mapVictronToBatterySystem(victronData) {
  if (!victronData || typeof victronData !== "object") {
    return {
      nodes: { ...EMPTY_NODES },
      wireAmps: {},
      batteryLevel: null,
      batteryFlow: computeFlowFromSmartShunt(null),
    };
  }

  const publishedAt = victronData.publishedAt;
  const shunt = getFreshVictronDevice(victronData.smartshunt, publishedAt);
  const mppt1 = getFreshVictronDevice(victronData.mppt1, publishedAt);
  const mppt2 = getFreshVictronDevice(victronData.mppt2, publishedAt);
  const orion = getFreshVictronDevice(victronData.orion, publishedAt);
  const acCharger = getFreshVictronDevice(victronData.acCharger, publishedAt);

  const batteryVoltage =
    shunt?.voltage ?? mppt1?.batteryVoltage ?? mppt2?.batteryVoltage ?? 12.6;

  const mppt1BatteryCurrent = Number(mppt1?.batteryCurrent) || 0;
  const mppt2BatteryCurrent = Number(mppt2?.batteryCurrent) || 0;
  const orionOutputCurrent = Number(orion?.outputCurrent) || 0;
  const acCurrent = Number(acCharger?.current) || 0;
  const shuntCurrent = shunt?.current != null ? Number(shunt.current) : null;

  let dcLoadsCurrent = null;
  if (shuntCurrent != null) {
    dcLoadsCurrent = round2(
      mppt1BatteryCurrent +
        mppt2BatteryCurrent +
        orionOutputCurrent +
        acCurrent -
        shuntCurrent
    );
  }

  const nodes = {
    charger230v: acCharger
      ? { voltage: 230, current: acCharger.current ?? 0 }
      : null,
    dcDcBooster: orion
      ? { voltage: orion.outputVoltage, current: orion.outputCurrent ?? 0 }
      : null,
    alternator: orion
      ? { voltage: orion.inputVoltage, current: orion.inputCurrent ?? 0 }
      : null,
    solarController1: mppt1
      ? { power: mppt1.pvPower ?? 0, voltage: mppt1.batteryVoltage }
      : null,
    solarController2: mppt2
      ? { power: mppt2.pvPower ?? 0, voltage: mppt2.batteryVoltage }
      : null,
    solarPanelGroup1: mppt1
      ? { power: mppt1.pvPower ?? 0, voltage: mppt1.batteryVoltage }
      : null,
    solarPanelGroup2: mppt2
      ? { power: mppt2.pvPower ?? 0, voltage: mppt2.batteryVoltage }
      : null,
    dcLoads:
      dcLoadsCurrent != null
        ? {
            power: Math.round(Math.max(0, dcLoadsCurrent) * batteryVoltage),
            voltage: batteryVoltage,
            current: Math.max(0, dcLoadsCurrent),
          }
        : null,
  };

  const wireAmps = {
    "solar1-mppt1": estimatePanelAmps(mppt1),
    "solar2-mppt2": estimatePanelAmps(mppt2),
    "alternator-dcdc": Number(orion?.inputCurrent) || 0,
    "dcdc-battery": orionOutputCurrent,
    "mppt1-battery": mppt1BatteryCurrent,
    "mppt2-battery": mppt2BatteryCurrent,
    "loads-battery": dcLoadsCurrent != null ? Math.max(0, dcLoadsCurrent) : 0,
    "ac-battery": acCurrent,
  };

  return {
    nodes,
    wireAmps,
    batteryLevel: shunt?.soc ?? null,
    batteryFlow: computeFlowFromSmartShunt(shunt),
  };
}
