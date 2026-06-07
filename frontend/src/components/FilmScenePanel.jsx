/**
 * Film scene — full-modal action picker.
 */

import {
  SceneActionPanel,
  SceneDelayOptionRow,
  SceneOptionRow,
  SCENE_DELAY_DEFAULT_SECONDS,
} from "./SceneActionPanel";

export const DEFAULT_FILM_OPTIONS = {
  audioOn: true,
  kitchenOff: true,
  mainOn: true,
  delay: false,
  delaySeconds: SCENE_DELAY_DEFAULT_SECONDS,
};

/**
 * @typedef {Object} FilmScenePayload
 * @property {boolean} audioOn
 * @property {boolean} kitchenOff
 * @property {boolean} mainOn
 */

/**
 * @param {typeof DEFAULT_FILM_OPTIONS} options
 * @returns {FilmScenePayload}
 */
export function buildFilmScenePayload(options) {
  return {
    audioOn: options.audioOn,
    kitchenOff: options.kitchenOff,
    mainOn: options.mainOn,
  };
}

/**
 * @param {Object} props
 * @param {(payload: FilmScenePayload) => void} props.onApply
 * @param {() => void} props.onBack
 */
export function FilmScenePanel({ onApply, onBack }) {
  return (
    <SceneActionPanel
      title="Cinema"
      defaultOptions={DEFAULT_FILM_OPTIONS}
      buildPayload={buildFilmScenePayload}
      onApply={onApply}
      onBack={onBack}
      renderOptions={({ options, setOption, onChange }) => (
        <>
          <SceneOptionRow checked={options.audioOn} onChange={(v) => setOption("audioOn", v)}>
            Audio on
          </SceneOptionRow>
          <SceneOptionRow checked={options.kitchenOff} onChange={(v) => setOption("kitchenOff", v)}>
            Kitchen off
          </SceneOptionRow>
          <SceneOptionRow checked={options.mainOn} onChange={(v) => setOption("mainOn", v)} spanAll>
            Main warm 10%
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
