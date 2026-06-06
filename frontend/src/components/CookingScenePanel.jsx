/**
 * Cooking scene — full-modal action picker.
 */

import {
  SceneActionPanel,
  SceneDelayOptionRow,
  SceneOptionRow,
  SCENE_DELAY_DEFAULT_SECONDS,
} from "./SceneActionPanel";

export const DEFAULT_COOKING_OPTIONS = {
  inverterOn: true,
  kitchenOn: true,
  delay: false,
  delaySeconds: SCENE_DELAY_DEFAULT_SECONDS,
};

/**
 * @typedef {Object} CookingScenePayload
 * @property {boolean} inverterOn
 * @property {boolean} kitchenOn
 */

/**
 * @param {typeof DEFAULT_COOKING_OPTIONS} options
 * @returns {CookingScenePayload}
 */
export function buildCookingScenePayload(options) {
  return {
    inverterOn: options.inverterOn,
    kitchenOn: options.kitchenOn,
  };
}

/**
 * @param {Object} props
 * @param {(payload: CookingScenePayload) => void} props.onApply
 * @param {() => void} props.onBack
 */
export function CookingScenePanel({ onApply, onBack }) {
  return (
    <SceneActionPanel
      title="Cooking"
      defaultOptions={DEFAULT_COOKING_OPTIONS}
      buildPayload={buildCookingScenePayload}
      onApply={onApply}
      onBack={onBack}
      renderOptions={({ options, setOption, onChange }) => (
        <>
          <SceneOptionRow checked={options.inverterOn} onChange={(v) => setOption("inverterOn", v)}>
            Inverter on
          </SceneOptionRow>
          <SceneOptionRow checked={options.kitchenOn} onChange={(v) => setOption("kitchenOn", v)}>
            Kitchen neutral 70%
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
