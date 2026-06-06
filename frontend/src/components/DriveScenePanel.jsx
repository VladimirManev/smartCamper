/**
 * Drive scene — full-modal action picker.
 */

import {
  SceneActionPanel,
  SceneDelayOptionRow,
  SceneOptionRow,
  SCENE_DELAY_DEFAULT_SECONDS,
} from "./SceneActionPanel";

export { SCENE_DELAY_OPTIONS as DRIVE_DELAY_OPTIONS, SCENE_DELAY_DEFAULT_SECONDS as DRIVE_DELAY_DEFAULT_SECONDS } from "./SceneActionPanel";

export const DEFAULT_DRIVE_OPTIONS = {
  inverterOff: true,
  boilerOff: true,
  lightsOff: true,
  radiantOff: true,
  pumpOff: true,
  audioOff: true,
  delay: false,
  delaySeconds: SCENE_DELAY_DEFAULT_SECONDS,
};

/**
 * @typedef {Object} DriveScenePayload
 * @property {boolean} inverterOff
 * @property {boolean} boilerOff
 * @property {boolean} lightsOff
 * @property {boolean} radiantOff
 * @property {boolean} pumpOff
 * @property {boolean} audioOff
 */

/**
 * @param {typeof DEFAULT_DRIVE_OPTIONS} options
 * @returns {DriveScenePayload}
 */
export function buildDriveScenePayload(options) {
  return {
    inverterOff: options.inverterOff,
    boilerOff: options.boilerOff,
    lightsOff: options.lightsOff,
    radiantOff: options.radiantOff,
    pumpOff: options.pumpOff,
    audioOff: options.audioOff,
  };
}

/**
 * @param {Object} props
 * @param {(payload: DriveScenePayload) => void} props.onApply
 * @param {() => void} props.onBack
 */
export function DriveScenePanel({ onApply, onBack }) {
  return (
    <SceneActionPanel
      title="Drive"
      defaultOptions={DEFAULT_DRIVE_OPTIONS}
      buildPayload={buildDriveScenePayload}
      onApply={onApply}
      onBack={onBack}
      renderOptions={({ options, setOption, onChange }) => (
        <>
          <SceneOptionRow checked={options.inverterOff} onChange={(v) => setOption("inverterOff", v)}>
            Inverter off
          </SceneOptionRow>
          <SceneOptionRow checked={options.boilerOff} onChange={(v) => setOption("boilerOff", v)}>
            Boiler off
          </SceneOptionRow>
          <SceneOptionRow checked={options.lightsOff} onChange={(v) => setOption("lightsOff", v)}>
            Lights off
          </SceneOptionRow>
          <SceneOptionRow checked={options.radiantOff} onChange={(v) => setOption("radiantOff", v)}>
            Radiant off
          </SceneOptionRow>
          <SceneOptionRow checked={options.pumpOff} onChange={(v) => setOption("pumpOff", v)}>
            Pump off
          </SceneOptionRow>
          <SceneOptionRow checked={options.audioOff} onChange={(v) => setOption("audioOff", v)}>
            Audio off
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
