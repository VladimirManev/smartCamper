/**
 * Battery energy diagram — nodes and wire link definitions.
 *
 * Topology:
 *   Solar 1 → MPPT 1 → Battery
 *   Solar 2 → MPPT 2 → Battery
 *   Alternator → DC-DC → Battery
 *   AC Charger → Battery (elbow path, 2 bends)
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

/** Victron payload key that feeds each diagram node (for stale / offline badges). */
export const BATTERY_NODE_VICTRON_SOURCE = {
  charger230v: "acCharger",
  dcDcBooster: "orion",
  alternator: "orion",
  solarController1: "mppt1",
  solarController2: "mppt2",
  solarPanelGroup1: "mppt1",
  solarPanelGroup2: "mppt2",
  dcLoads: "smartshunt",
};

/** Diagram nodes without OFF badge (display-only; stale state handled on linked device card). */
export const BATTERY_NODES_WITHOUT_OFF_BADGE = new Set([
  "alternator",
  "solarPanelGroup1",
  "solarPanelGroup2",
  "dcLoads",
]);

/** Wires labeled in watts (PV power) instead of amps. */
export const BATTERY_WIRE_POWER_LABELS = new Set(["solar1-mppt1", "solar2-mppt2"]);

/** Victron payload key per wire (for suppressing live flow animation when stale). */
export const BATTERY_WIRE_VICTRON_SOURCE = {
  "solar1-mppt1": "mppt1",
  "solar2-mppt2": "mppt2",
  "alternator-dcdc": "orion",
  "dcdc-battery": "orion",
  "mppt1-battery": "mppt1",
  "mppt2-battery": "mppt2",
  "loads-battery": "smartshunt",
  "ac-battery": "acCharger",
};

/** @typedef {'voltageCurrent' | 'powerVoltage'} BatteryNodeMetricKind */
/** @typedef {'top' | 'bottom' | 'left' | 'right'} BatteryWireEdge */

/**
 * @type {Array<{
 *   id: string;
 *   label: string;
 *   icon: string;
 *   image?: string;
 *   deviceLabel?: string;
 *   largeIcon?: boolean;
 *   iconKind?: 'solar' | 'alternator' | 'dc-loads';
 *   iconStyle?: 'outline';
 *   slot: string;
 *   metric: BatteryNodeMetricKind;
 * }>}
 */
export const BATTERY_SYSTEM_NODES = [
  {
    id: "charger230v",
    label: "AC Charger",
    icon: "fa-plug",
    largeIcon: true,
    iconStyle: "outline",
    slot: "r0c0",
    metric: "voltageCurrent",
  },
  {
    id: "dcDcBooster",
    label: "DC-DC",
    icon: "fa-right-left",
    deviceLabel: "DC-DC",
    slot: "r0c1",
    metric: "voltageCurrent",
  },
  {
    id: "alternator",
    label: "Alternator",
    icon: "fa-car",
    iconKind: "alternator",
    largeIcon: true,
    slot: "r0c2",
    metric: "voltageCurrent",
  },
  {
    id: "solarController1",
    label: "MPPT 1",
    icon: "fa-solar-panel",
    deviceLabel: "MPPT",
    slot: "r1c0",
    metric: "powerVoltage",
  },
  {
    id: "solarController2",
    label: "MPPT 2",
    icon: "fa-solar-panel",
    deviceLabel: "MPPT",
    slot: "r1c2",
    metric: "powerVoltage",
  },
  {
    id: "solarPanelGroup1",
    label: "Solar 1",
    icon: "fa-solar-panel",
    iconKind: "solar",
    largeIcon: true,
    slot: "r2c0",
    metric: "powerVoltage",
  },
  {
    id: "dcLoads",
    label: "DC Loads",
    icon: "fa-lightbulb",
    iconKind: "dc-loads",
    largeIcon: true,
    slot: "r2c1",
    metric: "powerVoltage",
  },
  {
    id: "solarPanelGroup2",
    label: "Solar 2",
    icon: "fa-solar-panel",
    iconKind: "solar",
    largeIcon: true,
    slot: "r2c2",
    metric: "powerVoltage",
  },
];

/**
 * Wire links — measured from card/battery shell edges at runtime.
 * route: "straight" (default) | "horizontal" | "elbow-to-top"
 *
 * @type {Array<{
 *   id: string;
 *   from: string;
 *   fromEdge: BatteryWireEdge;
 *   to: string;
 *   toEdge: BatteryWireEdge;
 *   route?: 'straight' | 'horizontal' | 'elbow-to-top';
 *   topAnchor?: number;
 *   elbowYRatio?: number;
 *   filletRadius?: number;
 *   alignVertical?: boolean;
 *   flow?: 'charge' | 'discharge';
 * }>}
 */
export const BATTERY_WIRE_LINKS = [
  {
    id: "solar1-mppt1",
    from: "solarPanelGroup1",
    fromEdge: "top",
    to: "solarController1",
    toEdge: "bottom",
    flow: "charge",
  },
  { id: "solar2-mppt2", from: "solarPanelGroup2", fromEdge: "top", to: "solarController2", toEdge: "bottom", flow: "charge" },
  { id: "alternator-dcdc", from: "alternator", fromEdge: "left", to: "dcDcBooster", toEdge: "right", flow: "charge" },
  { id: "dcdc-battery", from: "dcDcBooster", fromEdge: "bottom", to: "battery", toEdge: "top", topAnchor: 0.66, alignVertical: true, flow: "charge" },
  {
    id: "mppt1-battery",
    from: "solarController1",
    fromEdge: "right",
    to: "battery",
    toEdge: "left",
    route: "horizontal",
    flow: "charge",
  },
  {
    id: "mppt2-battery",
    from: "solarController2",
    fromEdge: "left",
    to: "battery",
    toEdge: "right",
    route: "horizontal",
    flow: "charge",
  },
  {
    id: "loads-battery",
    from: "dcLoads",
    fromEdge: "top",
    to: "battery",
    toEdge: "bottom",
    flow: "discharge",
  },
  {
    id: "ac-battery",
    from: "charger230v",
    fromEdge: "bottom",
    to: "battery",
    toEdge: "top",
    route: "elbow-to-top",
    topAnchor: 0.34,
    elbowYRatio: 0.5,
    filletRadius: 12,
    flow: "charge",
  },
];
