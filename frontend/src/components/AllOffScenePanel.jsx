/**
 * All Off scene — full-modal action picker (turn everything off).
 */

import {
  SceneActionPanel,
  SceneDelayOptionRow,
  SceneOptionRow,
  SCENE_DELAY_DEFAULT_SECONDS,
} from "./SceneActionPanel";

export const DEFAULT_ALL_OFF_OPTIONS = {
  lightsOff: true,
  radiantOff: true,
  inverterOff: true,
  boilerOff: true,
  pumpOff: true,
  audioOff: true,
  fridgeOff: true,
  wcFanOff: true,
  delay: false,
  delaySeconds: SCENE_DELAY_DEFAULT_SECONDS,
};

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
 * @param {typeof DEFAULT_ALL_OFF_OPTIONS} options
 * @returns {AllOffScenePayload}
 */
export function buildAllOffScenePayload(options) {
  return {
    lightsOff: options.lightsOff,
    radiantOff: options.radiantOff,
    inverterOff: options.inverterOff,
    boilerOff: options.boilerOff,
    pumpOff: options.pumpOff,
    audioOff: options.audioOff,
    fridgeOff: options.fridgeOff,
    wcFanOff: options.wcFanOff,
  };
}

/**
 * @param {Object} props
 * @param {(payload: AllOffScenePayload) => void} props.onApply
 * @param {() => void} props.onBack
 */
export function AllOffScenePanel({ onApply, onBack }) {
  return (
    <SceneActionPanel
      title="All Off"
      defaultOptions={DEFAULT_ALL_OFF_OPTIONS}
      buildPayload={buildAllOffScenePayload}
      onApply={onApply}
      onBack={onBack}
      renderOptions={({ options, setOption, onChange }) => (
        <>
          <SceneOptionRow checked={options.lightsOff} onChange={(v) => setOption("lightsOff", v)}>
            Lights off
          </SceneOptionRow>
          <SceneOptionRow checked={options.radiantOff} onChange={(v) => setOption("radiantOff", v)}>
            Radiant off
          </SceneOptionRow>
          <SceneOptionRow checked={options.inverterOff} onChange={(v) => setOption("inverterOff", v)}>
            Inverter off
          </SceneOptionRow>
          <SceneOptionRow checked={options.boilerOff} onChange={(v) => setOption("boilerOff", v)}>
            Boiler off
          </SceneOptionRow>
          <SceneOptionRow checked={options.pumpOff} onChange={(v) => setOption("pumpOff", v)}>
            Pump off
          </SceneOptionRow>
          <SceneOptionRow checked={options.audioOff} onChange={(v) => setOption("audioOff", v)}>
            Audio off
          </SceneOptionRow>
          <SceneOptionRow checked={options.fridgeOff} onChange={(v) => setOption("fridgeOff", v)}>
            Fridge off
          </SceneOptionRow>
          <SceneOptionRow checked={options.wcFanOff} onChange={(v) => setOption("wcFanOff", v)}>
            WC Fan off
          </SceneOptionRow>
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
