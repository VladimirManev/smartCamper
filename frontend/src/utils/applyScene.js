/**
 * Apply scene presets — dispatches commands to lighting and appliances.
 */

import { LED_WHITE_PRESETS } from "../components/LEDStripModalContent";
import { SCENE_REQUIRED_MODULES } from "../constants/scenes";
import {
  AMBIENT_RELAY_INDEX,
  LIGHTING_GROUP_STRIP_INDICES,
  getLightingMasterOffPlan,
  isStripActiveForLightingGroup,
} from "./lightingGroupAggregate";
import { getRadiantMasterOffPlan } from "./radiantGroupAggregate";
import { APPLIANCE_INDEX } from "../constants/appliances";

const STRIP_INDEX = {
  kitchen: 0,
  main: 1,
  bathroom: 3,
  bedroom: 4,
};

export const SLEEP_BEDROOM_OFF_DEFAULT_SECONDS = 120;

function brightnessPercent(percent) {
  return Math.max(1, Math.round((255 * percent) / 100));
}

function ensureApplianceOn(appliances, index, sendApplianceCommand) {
  if (appliances[index]?.state !== "ON") {
    sendApplianceCommand({
      type: "relay",
      index,
      action: "toggle",
    });
  }
}

function ensureApplianceOff(appliances, index, sendApplianceCommand) {
  if (appliances[index]?.state === "ON") {
    sendApplianceCommand({
      type: "relay",
      index,
      action: "toggle",
    });
  }
}

/** Inverter OFF always turns boiler OFF first. */
function ensureInverterOff(appliances, sendApplianceCommand) {
  ensureApplianceOff(appliances, APPLIANCE_INDEX.boiler, sendApplianceCommand);
  ensureApplianceOff(appliances, APPLIANCE_INDEX.inverter, sendApplianceCommand);
}

/**
 * @typedef {Object} NormalScenePayload
 * @property {boolean} fridgeOn
 * @property {boolean} pumpOn
 * @property {boolean} wcFanOn
 * @property {boolean} mainOn
 */

/**
 * Normal: fridge, pump, WC fan ON; main neutral white @ 50%.
 *
 * @param {Object} ctx
 * @param {NormalScenePayload} options
 */
export function applyNormalScene({ sendLEDCommand, sendApplianceCommand, appliances }, options) {
  if (options.fridgeOn) {
    ensureApplianceOn(appliances, APPLIANCE_INDEX.fridge, sendApplianceCommand);
  }

  if (options.pumpOn) {
    ensureApplianceOn(appliances, APPLIANCE_INDEX.pump, sendApplianceCommand);
  }

  if (options.wcFanOn) {
    ensureApplianceOn(appliances, APPLIANCE_INDEX.wcFan, sendApplianceCommand);
  }

  if (options.mainOn) {
    sendLEDCommand({
      type: "strip",
      index: STRIP_INDEX.main,
      action: "apply",
      payload: {
        brightness: brightnessPercent(50),
        mode: "on",
        state: "ON",
        effect: "normal",
        channels: { ...LED_WHITE_PRESETS.neutral },
      },
    });
  }
}

/**
 * Film: optional audio ON, kitchen OFF, main warm white @ 10%.
 *
 * @param {Object} ctx
 * @param {FilmScenePayload} options
 */
export function applyFilmScene({ sendLEDCommand, sendApplianceCommand, appliances }, options) {
  if (options.audioOn) {
    ensureApplianceOn(appliances, APPLIANCE_INDEX.audio, sendApplianceCommand);
  }

  if (options.kitchenOff) {
    sendLEDCommand({
      type: "strip",
      index: STRIP_INDEX.kitchen,
      action: "off",
    });
  }

  if (options.mainOn) {
    sendLEDCommand({
      type: "strip",
      index: STRIP_INDEX.main,
      action: "apply",
      payload: {
        brightness: brightnessPercent(10),
        mode: "on",
        state: "ON",
        effect: "normal",
        channels: { ...LED_WHITE_PRESETS.warm },
      },
    });
  }
}

/**
 * Cooking: optional inverter ON, kitchen neutral white @ 70%.
 *
 * @param {Object} ctx
 * @param {CookingScenePayload} options
 */
export function applyCookingScene({ sendLEDCommand, sendApplianceCommand, appliances }, options) {
  if (options.inverterOn) {
    ensureApplianceOn(appliances, APPLIANCE_INDEX.inverter, sendApplianceCommand);
  }

  if (options.kitchenOn) {
    sendLEDCommand({
      type: "strip",
      index: STRIP_INDEX.kitchen,
      action: "apply",
      payload: {
        brightness: brightnessPercent(70),
        mode: "on",
        state: "ON",
        effect: "normal",
        channels: { ...LED_WHITE_PRESETS.neutral },
      },
    });
  }
}

/**
 * @typedef {Object} CookingScenePayload
 * @property {boolean} inverterOn
 * @property {boolean} kitchenOn
 */

/**
 * Drive: apply selected off actions (lights, appliances, radiant).
 *
 * @param {Object} ctx
 * @param {DriveScenePayload} options
 */
export function applyDriveScene(
  {
    sendLEDCommand,
    sendApplianceCommand,
    sendFloorHeatingCommand,
    ledStrips,
    relays,
    appliances,
    circles,
  },
  options
) {
  if (options.lightsOff) {
    const { stripIndices, toggleAmbient } = getLightingMasterOffPlan(ledStrips, relays);
    for (const index of stripIndices) {
      sendLEDCommand({ type: "strip", index, action: "off" });
    }
    if (toggleAmbient) {
      sendLEDCommand({ type: "relay", action: "toggle" });
    }
  }

  if (options.pumpOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.pump, sendApplianceCommand);
  }

  if (options.audioOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.audio, sendApplianceCommand);
  }

  if (options.radiantOff) {
    const { indicesToOff } = getRadiantMasterOffPlan(circles);
    for (const index of indicesToOff) {
      sendFloorHeatingCommand({ type: "circle", index, action: "off" });
    }
  }

  if (options.boilerOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.boiler, sendApplianceCommand);
  }

  if (options.inverterOff) {
    ensureInverterOff(appliances, sendApplianceCommand);
  }
}

/** All Off: every lighting-group strip that is ON (includes bathroom AUTO). */
function getAllLightsOffPlan(ledStrips, relays) {
  const stripIndices = [];
  for (const index of LIGHTING_GROUP_STRIP_INDICES) {
    if (ledStrips[index]?.state === "ON") {
      stripIndices.push(index);
    }
  }
  const toggleAmbient = relays?.[AMBIENT_RELAY_INDEX]?.state === "ON";
  return { stripIndices, toggleAmbient };
}

/**
 * @typedef {Object} AllOffScenePayload
 * @property {boolean} lightsOff
 * @property {boolean} radiantOff
 * @property {boolean} inverterOff
 * @property {boolean} boilerOff
 * @property {boolean} pumpOff
 * @property {boolean} audioOff
 * @property {boolean} fridgeOff
 * @property {boolean} wcFanOff
 */

/**
 * All Off: turn off selected lights, radiant zones, and appliances.
 *
 * @param {Object} ctx
 * @param {AllOffScenePayload} options
 */
export function applyAllOffScene(
  {
    sendLEDCommand,
    sendApplianceCommand,
    sendFloorHeatingCommand,
    ledStrips,
    relays,
    appliances,
    circles,
  },
  options
) {
  if (options.lightsOff) {
    const { stripIndices, toggleAmbient } = getAllLightsOffPlan(ledStrips, relays);
    for (const index of stripIndices) {
      sendLEDCommand({ type: "strip", index, action: "off" });
    }
    if (toggleAmbient) {
      sendLEDCommand({ type: "relay", action: "toggle" });
    }
  }

  if (options.radiantOff) {
    const { indicesToOff } = getRadiantMasterOffPlan(circles);
    for (const index of indicesToOff) {
      sendFloorHeatingCommand({ type: "circle", index, action: "off" });
    }
  }

  if (options.pumpOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.pump, sendApplianceCommand);
  }

  if (options.audioOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.audio, sendApplianceCommand);
  }

  if (options.fridgeOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.fridge, sendApplianceCommand);
  }

  if (options.wcFanOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.wcFan, sendApplianceCommand);
  }

  if (options.boilerOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.boiler, sendApplianceCommand);
  }

  if (options.inverterOff) {
    ensureInverterOff(appliances, sendApplianceCommand);
  }
}

/**
 * @typedef {Object} SleepScenePayload
 * @property {boolean} pumpOff
 * @property {boolean} audioOff
 * @property {boolean} boilerOff
 * @property {boolean} inverterOff
 * @property {boolean} lightsOffExceptBedroom
 * @property {boolean} bathroomAuto10
 * @property {boolean} bedroomDimThenOff
 * @property {number} bedroomOffSeconds
 */

function getSleepLightsOffExceptBedroomPlan(ledStrips, relays, skipBathroom = true) {
  const stripIndices = [];
  for (const index of LIGHTING_GROUP_STRIP_INDICES) {
    if (index === STRIP_INDEX.bedroom) continue;
    if (index === STRIP_INDEX.bathroom && skipBathroom) continue;
    if (isStripActiveForLightingGroup(index, ledStrips[index])) {
      stripIndices.push(index);
    }
  }
  const toggleAmbient = relays?.[AMBIENT_RELAY_INDEX]?.state === "ON";
  return { stripIndices, toggleAmbient };
}

/**
 * Sleep: off appliances/lights (except bedroom + bathroom), bathroom AUTO @ 10%, bedroom @ 60%, then bedroom off after chosen delay.
 * Returns cleanup to cancel the bedroom off timer (e.g. when re-applying Sleep).
 *
 * @param {Object} ctx
 * @param {SleepScenePayload} options
 * @returns {() => void}
 */
export function applySleepScene(
  {
    sendLEDCommand,
    sendApplianceCommand,
    ledStrips,
    relays,
    appliances,
  },
  options
) {
  if (options.pumpOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.pump, sendApplianceCommand);
  }

  if (options.audioOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.audio, sendApplianceCommand);
  }

  if (options.boilerOff) {
    ensureApplianceOff(appliances, APPLIANCE_INDEX.boiler, sendApplianceCommand);
  }

  if (options.inverterOff) {
    ensureInverterOff(appliances, sendApplianceCommand);
  }

  if (options.lightsOffExceptBedroom) {
    const { stripIndices, toggleAmbient } = getSleepLightsOffExceptBedroomPlan(
      ledStrips,
      relays,
      options.bathroomAuto10
    );
    for (const index of stripIndices) {
      sendLEDCommand({ type: "strip", index, action: "off" });
    }
    if (toggleAmbient) {
      sendLEDCommand({ type: "relay", action: "toggle" });
    }
  }

  if (options.bathroomAuto10) {
    sendLEDCommand({
      type: "strip",
      index: STRIP_INDEX.bathroom,
      action: "apply",
      payload: {
        brightness: brightnessPercent(10),
        mode: "auto",
      },
    });
  }

  let bedroomOffTimer = null;

  if (options.bedroomDimThenOff) {
    sendLEDCommand({
      type: "strip",
      index: STRIP_INDEX.bedroom,
      action: "apply",
      payload: {
        brightness: brightnessPercent(60),
        mode: "on",
        state: "ON",
        effect: "normal",
        channels: { ...LED_WHITE_PRESETS.warm },
      },
    });

    const offDelayMs = Math.max(0, options.bedroomOffSeconds ?? SLEEP_BEDROOM_OFF_DEFAULT_SECONDS) * 1000;

    bedroomOffTimer = window.setTimeout(() => {
      sendLEDCommand({ type: "strip", index: STRIP_INDEX.bedroom, action: "off" });
    }, offDelayMs);
  }

  return () => {
    if (bedroomOffTimer !== null) {
      window.clearTimeout(bedroomOffTimer);
    }
  };
}

/**
 * @param {string} sceneId
 * @param {Object} ctx
 */
export function applyScene(sceneId, ctx) {
  switch (sceneId) {
    default:
      break;
  }
}

/**
 * @param {string} sceneId
 * @param {(moduleId: string) => boolean} isModuleOnline
 */
export function isSceneDisabled(sceneId, isModuleOnline) {
  const required = SCENE_REQUIRED_MODULES[sceneId];
  if (!required?.length) return false;
  return required.some((moduleId) => !isModuleOnline(moduleId));
}
