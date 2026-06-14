/**
 * Sleep scene — full-modal action picker.
 */

import {
  SceneActionPanel,
  SceneDelayOptionRow,
  SceneOptionRow,
  SceneStepperOptionRow,
  SCENE_DELAY_DEFAULT_SECONDS,
} from "./SceneActionPanel";

/** Bedroom off timer: 1, 2, 5, 10 minutes (seconds). */
export const SLEEP_BEDROOM_OFF_OPTIONS = [60, 120, 300, 600];
export const SLEEP_BEDROOM_OFF_DEFAULT_SECONDS = 120;

function formatBedroomOffDuration(seconds) {
  return `${seconds / 60}m`;
}

export const DEFAULT_SLEEP_OPTIONS = {
  pumpOff: true,
  audioOff: true,
  boilerOff: true,
  inverterOff: true,
  lightsOffExceptBedroom: true,
  bathroomAuto10: true,
  bedroomDimThenOff: true,
  bedroomOffSeconds: SLEEP_BEDROOM_OFF_DEFAULT_SECONDS,
  delay: false,
  delaySeconds: SCENE_DELAY_DEFAULT_SECONDS,
};

/**
 * @typedef {import("../utils/applyScene").SleepScenePayload} SleepScenePayload
 */

/**
 * @param {typeof DEFAULT_SLEEP_OPTIONS} options
 * @returns {SleepScenePayload}
 */
export function buildSleepScenePayload(options) {
  return {
    pumpOff: options.pumpOff,
    audioOff: options.audioOff,
    boilerOff: options.boilerOff,
    inverterOff: options.inverterOff,
    lightsOffExceptBedroom: options.lightsOffExceptBedroom,
    bathroomAuto10: options.bathroomAuto10,
    bedroomDimThenOff: options.bedroomDimThenOff,
    bedroomOffSeconds: options.bedroomOffSeconds,
  };
}

/**
 * @param {Object} props
 * @param {(payload: SleepScenePayload) => void} props.onApply
 * @param {() => void} props.onBack
 */
export function SleepScenePanel({ onApply, onBack }) {
  return (
    <SceneActionPanel
      title="Sleep"
      defaultOptions={DEFAULT_SLEEP_OPTIONS}
      buildPayload={buildSleepScenePayload}
      onApply={onApply}
      onBack={onBack}
      renderOptions={({ options, setOption, onChange }) => (
        <>
          <SceneOptionRow checked={options.pumpOff} onChange={(v) => setOption("pumpOff", v)}>
            Pump off
          </SceneOptionRow>
          <SceneOptionRow checked={options.audioOff} onChange={(v) => setOption("audioOff", v)}>
            Audio off
          </SceneOptionRow>
          <SceneOptionRow checked={options.boilerOff} onChange={(v) => setOption("boilerOff", v)}>
            Boiler off
          </SceneOptionRow>
          <SceneOptionRow checked={options.inverterOff} onChange={(v) => setOption("inverterOff", v)}>
            Inverter off
          </SceneOptionRow>
          <SceneOptionRow
            checked={options.lightsOffExceptBedroom}
            onChange={(v) => setOption("lightsOffExceptBedroom", v)}
          >
            Lights off
          </SceneOptionRow>
          <SceneOptionRow
            checked={options.bathroomAuto10}
            onChange={(v) => setOption("bathroomAuto10", v)}
          >
            Bath AUTO 10%
          </SceneOptionRow>
          <SceneStepperOptionRow
            checked={options.bedroomDimThenOff}
            onCheckedChange={(checked) => setOption("bedroomDimThenOff", checked)}
            value={options.bedroomOffSeconds}
            onValueChange={(bedroomOffSeconds) => onChange({ ...options, bedroomOffSeconds })}
            stepOptions={SLEEP_BEDROOM_OFF_OPTIONS}
            label="Bedroom 60% → off"
            formatValue={formatBedroomOffDuration}
            decreaseAriaLabel="Decrease bedroom off time"
            increaseAriaLabel="Increase bedroom off time"
          />
          <SceneDelayOptionRow
            checked={options.delay}
            delaySeconds={options.delaySeconds}
            onCheckedChange={(checked) => setOption("delay", checked)}
            onSecondsChange={(delaySeconds) => onChange({ ...options, delaySeconds })}
          />
        </>
      )}
    />
  );
}
