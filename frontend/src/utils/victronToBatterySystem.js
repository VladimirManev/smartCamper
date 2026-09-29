/**
 * Map module-6 Victron MQTT/WebSocket payload to battery diagram nodes and wires.
 */

import {
  BATTERY_NODE_IDS,
  BATTERY_NODE_VICTRON_SOURCE,
  BATTERY_WIRE_VICTRON_SOURCE,
} from "../config/batterySystemNodes";
import {
  BATTERY_CHARGER_PHASE_NODES,
  formatVictronDeviceState,
} from "./victronDeviceState";

export const VICTRON_STALE_MS = 6000;

const WIRE_MIN_VISIBLE_AMPS = 0.05;

const EMPTY_NODES = Object.fromEntries(BATTERY_NODE_IDS.map((id) => [id, null]));

const VICTRON_KEYS = ["smartshunt", "mppt1", "mppt2", "orion", "acCharger"];

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
 * @returns {{ device: Object|null, isOffline: boolean }}
 */
export function resolveVictronDevice(device, publishedAt) {
  if (!device || typeof device !== "object") {
    return { device: null, isOffline: true };
  }
  return {
    device,
    isOffline: isVictronDeviceStale(publishedAt, device.updatedAt),
  };
}

function round2(value) {
  return Math.round(Number(value) * 100) / 100;
}

/**
 * @param {Object|null|undefined} shunt
 * @param {number|null|undefined} [fallbackVoltage]
 */
export function computeFlowFromSmartShunt(shunt, fallbackVoltage = null) {
  const resolvedFallback =
    fallbackVoltage != null && !Number.isNaN(Number(fallbackVoltage))
      ? round2(fallbackVoltage)
      : null;

  const voltage =
    shunt?.voltage != null && !Number.isNaN(Number(shunt.voltage))
      ? round2(shunt.voltage)
      : resolvedFallback;

  if (!shunt || shunt.current == null) {
    return { direction: "idle", amps: 0, watts: 0, netAmps: 0, voltage };
  }

  const netAmps = round2(shunt.current);
  const flowVoltage = voltage ?? 12.6;
  const absAmps = Math.round(Math.abs(netAmps) * 10) / 10;
  const watts = Math.round(absAmps * flowVoltage);

  let direction = "idle";
  if (netAmps > WIRE_MIN_VISIBLE_AMPS) direction = "charge";
  else if (netAmps < -WIRE_MIN_VISIBLE_AMPS) direction = "discharge";

  return { direction, amps: absAmps, watts, netAmps, voltage: flowVoltage };
}

function buildOfflineBySource(victronSources) {
  /** @type {Record<string, boolean>} */
  const offlineBySource = {};
  for (const key of VICTRON_KEYS) {
    offlineBySource[key] = victronSources[key].isOffline;
  }
  return offlineBySource;
}

function buildOfflineByNode(offlineBySource) {
  /** @type {Record<string, boolean>} */
  const offlineByNode = {};
  for (const nodeId of BATTERY_NODE_IDS) {
    const source = BATTERY_NODE_VICTRON_SOURCE[nodeId];
    offlineByNode[nodeId] = offlineBySource[source] ?? true;
  }
  return offlineByNode;
}

function buildOfflineByWire(offlineBySource) {
  /** @type {Record<string, boolean>} */
  const offlineByWire = {};
  for (const [wireId, source] of Object.entries(BATTERY_WIRE_VICTRON_SOURCE)) {
    offlineByWire[wireId] = offlineBySource[source] ?? true;
  }
  return offlineByWire;
}

/**
 * Phase badge text per charger node (null when unknown / not a charger).
 * Offline → OFF; live → mapped `deviceState`.
 * @param {Record<string, { device: Object|null, isOffline: boolean }>} victronSources
 */
function buildPhaseByNode(victronSources) {
  /** @type {Record<string, string|null>} */
  const phaseByNode = {};
  for (const nodeId of BATTERY_CHARGER_PHASE_NODES) {
    const sourceKey = BATTERY_NODE_VICTRON_SOURCE[nodeId];
    const source = victronSources[sourceKey];
    if (!source || source.isOffline || !source.device) {
      phaseByNode[nodeId] = "OFF";
      continue;
    }
    phaseByNode[nodeId] =
      formatVictronDeviceState(source.device.deviceState) ?? "OFF";
  }
  return phaseByNode;
}

/**
 * @param {Object|null|undefined} victronData
 * @returns {{
 *   nodes: Object,
 *   wireAmps: Object,
 *   batteryLevel: number|null,
 *   batteryFlow: Object,
 *   batteryVoltage: number|null,
 *   batteryTemperature: number|null,
 *   offlineByNode: Record<string, boolean>,
 *   offlineByWire: Record<string, boolean>,
 *   phaseByNode: Record<string, string|null>,
 *   smartShuntOffline: boolean,
 * }}
 */
export function mapVictronToBatterySystem(victronData) {
  const emptyOfflineNodes = Object.fromEntries(
    BATTERY_NODE_IDS.map((id) => [id, true])
  );
  const emptyOfflineWires = Object.fromEntries(
    Object.keys(BATTERY_WIRE_VICTRON_SOURCE).map((id) => [id, true])
  );
  const emptyPhaseByNode = Object.fromEntries(
    BATTERY_CHARGER_PHASE_NODES.map((id) => [id, "OFF"])
  );

  if (!victronData || typeof victronData !== "object") {
    return {
      nodes: { ...EMPTY_NODES },
      wireAmps: {},
      batteryLevel: null,
      batteryFlow: computeFlowFromSmartShunt(null),
      batteryVoltage: null,
      batteryTemperature: null,
      offlineByNode: emptyOfflineNodes,
      offlineByWire: emptyOfflineWires,
      phaseByNode: emptyPhaseByNode,
      smartShuntOffline: true,
    };
  }

  const publishedAt = victronData.publishedAt;
  const victronSources = {
    smartshunt: resolveVictronDevice(victronData.smartshunt, publishedAt),
    mppt1: resolveVictronDevice(victronData.mppt1, publishedAt),
    mppt2: resolveVictronDevice(victronData.mppt2, publishedAt),
    orion: resolveVictronDevice(victronData.orion, publishedAt),
    acCharger: resolveVictronDevice(victronData.acCharger, publishedAt),
  };

  const live = (source) => (source.isOffline ? null : source.device);

  const shunt = live(victronSources.smartshunt);
  const mppt1 = live(victronSources.mppt1);
  const mppt2 = live(victronSources.mppt2);
  const orion = live(victronSources.orion);
  const acCharger = live(victronSources.acCharger);

  const offlineBySource = buildOfflineBySource(victronSources);
  const offlineByNode = buildOfflineByNode(offlineBySource);
  const offlineByWire = buildOfflineByWire(offlineBySource);
  const phaseByNode = buildPhaseByNode(victronSources);

  const batteryVoltage =
    shunt?.voltage != null
      ? round2(shunt.voltage)
      : mppt1?.batteryVoltage != null
        ? round2(mppt1.batteryVoltage)
        : mppt2?.batteryVoltage != null
          ? round2(mppt2.batteryVoltage)
          : null;

  const batteryTemperature =
    shunt?.temperature != null && !Number.isNaN(Number(shunt.temperature))
      ? round2(shunt.temperature)
      : null;

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

  const wireVoltage = batteryVoltage ?? 12.6;

  const nodes = {
    charger230v: acCharger
      ? {
          voltage: 230,
          current: acCharger.acCurrent ?? 0,
          batteryCurrent: acCharger.current ?? 0,
        }
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
            power: Math.round(Math.max(0, dcLoadsCurrent) * wireVoltage),
            voltage: wireVoltage,
            current: Math.max(0, dcLoadsCurrent),
          }
        : null,
  };

  const wireAmps = {
    "solar1-mppt1": Number(mppt1?.pvPower) || 0,
    "solar2-mppt2": Number(mppt2?.pvPower) || 0,
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
    batteryFlow: computeFlowFromSmartShunt(shunt, batteryVoltage),
    batteryVoltage,
    batteryTemperature,
    offlineByNode,
    offlineByWire,
    phaseByNode,
    smartShuntOffline: offlineBySource.smartshunt,
  };
}
