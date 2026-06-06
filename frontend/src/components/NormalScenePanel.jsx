/**
 * Normal scene — full-modal action picker.
 */

import {
  SceneActionPanel,
  SceneDelayOptionRow,
  SceneOptionRow,
  SCENE_DELAY_DEFAULT_SECONDS,
} from "./SceneActionPanel";

export const DEFAULT_NORMAL_OPTIONS = {
  fridgeOn: true,
  pumpOn: true,
  wcFanOn: true,
  mainOn: true,
  delay: false,
  delaySeconds: SCENE_DELAY_DEFAULT_SECONDS,
};

/**
 * @typedef {Object} NormalScenePayload
 * @property {boolean} fridgeOn
 * @property {boolean} pumpOn
 * @property {boolean} wcFanOn
 * @property {boolean} mainOn
 */

/**
 * @param {typeof DEFAULT_NORMAL_OPTIONS} options
 * @returns {NormalScenePayload}
 */
export function buildNormalScenePayload(options) {
  return {
    fridgeOn: options.fridgeOn,
    pumpOn: options.pumpOn,
    wcFanOn: options.wcFanOn,
    mainOn: options.mainOn,
  };
}

/**
 * @param {Object} props
 * @param {(payload: NormalScenePayload) => void} props.onApply
 * @param {() => void} props.onBack
 */
export function NormalScenePanel({ onApply, onBack }) {
  return (
    <SceneActionPanel
      title="Normal"
      defaultOptions={DEFAULT_NORMAL_OPTIONS}
      buildPayload={buildNormalScenePayload}
      onApply={onApply}
      onBack={onBack}
      renderOptions={({ options, setOption, onChange }) => (
        <>
          <SceneOptionRow checked={options.fridgeOn} onChange={(v) => setOption("fridgeOn", v)}>
            Fridge on
          </SceneOptionRow>
          <SceneOptionRow checked={options.pumpOn} onChange={(v) => setOption("pumpOn", v)}>
            Pump on
          </SceneOptionRow>
          <SceneOptionRow checked={options.wcFanOn} onChange={(v) => setOption("wcFanOn", v)}>
            WC Fan on
          </SceneOptionRow>
          <SceneOptionRow checked={options.mainOn} onChange={(v) => setOption("mainOn", v)} spanAll>
            Main neutral 50%
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
