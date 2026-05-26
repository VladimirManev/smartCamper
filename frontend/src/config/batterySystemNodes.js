/**
 * Battery energy diagram — nodes and wire paths (viewBox 0 0 100 100).
 *
 * Topology (matches reference diagram):
 *   Solar Panel 1 → Solar Controller 1 → Battery
 *   Solar Panel 2 → Solar Controller 2 → Battery
 *   230V Charger / DC-DC / Alternator → Battery
 *   Battery → DC Loads
 */

export const BATTERY_NODE_IDS = [
  "charger230v",
  "dcDcBooster",
  "alternator",
  "solarController1",
  "solarController2",
  "solarPanelGroup1",
  "solarPanelGroup2",
  "dcLoads",
];

/** @typedef {'voltageCurrent' | 'powerVoltage'} BatteryNodeMetricKind */

/**
 * @type {Array<{
 *   id: string;
 *   label: string;
 *   icon: string;
 *   slot: string;
 *   metric: BatteryNodeMetricKind;
 * }>}
 */
export const BATTERY_SYSTEM_NODES = [
  {
    id: "charger230v",
    label: "230V Charger",
    icon: "fa-plug",
    slot: "r0c0",
    metric: "voltageCurrent",
  },
  {
    id: "dcDcBooster",
    label: "DC-DC Booster",
    icon: "fa-right-left",
    slot: "r0c1",
    metric: "voltageCurrent",
  },
  {
    id: "alternator",
    label: "Alternator",
    icon: "fa-car",
    slot: "r0c2",
    metric: "voltageCurrent",
  },
  {
    id: "solarController1",
    label: "Solar Controller 1",
    icon: "fa-solar-panel",
    slot: "r1c0",
    metric: "powerVoltage",
  },
  {
    id: "solarController2",
    label: "Solar Controller 2",
    icon: "fa-solar-panel",
    slot: "r1c2",
    metric: "powerVoltage",
  },
  {
    id: "solarPanelGroup1",
    label: "Solar Panel Group 1",
    icon: "fa-sun",
    slot: "r2c0",
    metric: "powerVoltage",
  },
  {
    id: "dcLoads",
    label: "DC Loads",
    icon: "fa-lightbulb",
    slot: "r2c1",
    metric: "powerVoltage",
  },
  {
    id: "solarPanelGroup2",
    label: "Solar Panel Group 2",
    icon: "fa-sun",
    slot: "r2c2",
    metric: "powerVoltage",
  },
];

/** Connection points tuned to 3×3 grid (orthogonal paths). */
const P = {
  charger: { x: 17, y: 27 },
  dcdc: { x: 50, y: 27 },
  alternator: { x: 83, y: 27 },
  solar1: { x: 22, y: 50 },
  solar2: { x: 78, y: 50 },
  panels1: { x: 17, y: 73 },
  panels2: { x: 83, y: 73 },
  loads: { x: 50, y: 73 },
  battTop: { x: 50, y: 44 },
  battBottom: { x: 50, y: 56 },
  battLeft: { x: 44, y: 50 },
  battRight: { x: 56, y: 50 },
};

/**
 * @type {Array<{ id: string; d: string }>}
 */
export const BATTERY_WIRE_PATHS = [
  /* Solar chain 1: panels → controller → battery */
  { id: "panels1-solar1", d: `M ${P.panels1.x} ${P.panels1.y} L ${P.panels1.x} 61 L ${P.solar1.x} 61 L ${P.solar1.x} ${P.solar1.y}` },
  { id: "solar1-battery", d: `M ${P.solar1.x} ${P.solar1.y} L ${P.battLeft.x} ${P.battLeft.y}` },
  /* Solar chain 2 */
  { id: "panels2-solar2", d: `M ${P.panels2.x} ${P.panels2.y} L ${P.panels2.x} 61 L ${P.solar2.x} 61 L ${P.solar2.x} ${P.solar2.y}` },
  { id: "solar2-battery", d: `M ${P.solar2.x} ${P.solar2.y} L ${P.battRight.x} ${P.battRight.y}` },
  /* Top row chargers → battery */
  { id: "charger-battery", d: `M ${P.charger.x} ${P.charger.y} L ${P.charger.x} 40 L ${P.battLeft.x} 40 L ${P.battLeft.x} ${P.battTop.y}` },
  { id: "dcdc-battery", d: `M ${P.dcdc.x} ${P.dcdc.y} L ${P.dcdc.x} ${P.battTop.y}` },
  { id: "alternator-battery", d: `M ${P.alternator.x} ${P.alternator.y} L ${P.alternator.x} 40 L ${P.battRight.x} 40 L ${P.battRight.x} ${P.battTop.y}` },
  /* Battery → loads */
  { id: "battery-loads", d: `M ${P.battBottom.x} ${P.battBottom.y} L ${P.loads.x} ${P.loads.y}` },
];
