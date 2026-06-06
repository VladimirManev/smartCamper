/**
 * Shared full-modal scene action picker with optional delay countdown.
 */

import { useEffect, useRef, useState } from "react";

export const SCENE_DELAY_OPTIONS = [5, 10, 30, 60, 120];
export const SCENE_DELAY_DEFAULT_SECONDS = 10;

export function SceneOptionRow({ checked, onChange, children, spanAll = false }) {
  return (
    <label
      className={`scene-drive-option${checked ? " scene-drive-option--checked" : ""}${spanAll ? " scene-drive-option--span-all" : ""}`}
    >
      <input
        type="checkbox"
        className="scene-drive-option__input"
        checked={checked}
        onChange={(e) => onChange(e.target.checked)}
      />
      <span className="scene-drive-option__control" aria-hidden="true" />
      <span className="scene-drive-option__label">{children}</span>
    </label>
  );
}

export function SceneStepperOptionRow({
  checked,
  onCheckedChange,
  value,
  onValueChange,
  stepOptions,
  label,
  formatValue = (seconds) => `${seconds}s`,
  decreaseAriaLabel = "Decrease",
  increaseAriaLabel = "Increase",
}) {
  const stepIndex = stepOptions.indexOf(value);
  const safeIndex = stepIndex >= 0 ? stepIndex : 0;
  const canDecrement = safeIndex > 0;
  const canIncrement = safeIndex < stepOptions.length - 1;

  const stepValue = (direction) => {
    const nextIndex = safeIndex + direction;
    if (nextIndex < 0 || nextIndex >= stepOptions.length) return;
    onValueChange(stepOptions[nextIndex]);
  };

  return (
    <div className={`scene-drive-option scene-drive-option--delay scene-drive-option--span-all${checked ? " scene-drive-option--checked" : ""}`}>
      <label className="scene-drive-option__toggle">
        <input
          type="checkbox"
          className="scene-drive-option__input"
          checked={checked}
          onChange={(e) => onCheckedChange(e.target.checked)}
        />
        <span className="scene-drive-option__control" aria-hidden="true" />
        <span className="scene-drive-option__label">{label}</span>
      </label>
      <div className="scene-drive-delay-stepper">
        <button
          type="button"
          className="scene-drive-delay-stepper__btn"
          disabled={!canDecrement}
          aria-label={decreaseAriaLabel}
          onClick={() => stepValue(-1)}
        >
          −
        </button>
        <span className="scene-drive-delay-stepper__value" aria-live="polite">
          {formatValue(stepOptions[safeIndex])}
        </span>
        <button
          type="button"
          className="scene-drive-delay-stepper__btn"
          disabled={!canIncrement}
          aria-label={increaseAriaLabel}
          onClick={() => stepValue(1)}
        >
          +
        </button>
      </div>
    </div>
  );
}

export function SceneDelayOptionRow({ checked, delaySeconds, onCheckedChange, onSecondsChange }) {
  return (
    <SceneStepperOptionRow
      checked={checked}
      onCheckedChange={onCheckedChange}
      value={delaySeconds}
      onValueChange={onSecondsChange}
      stepOptions={SCENE_DELAY_OPTIONS}
      label="Delay"
      formatValue={(seconds) => `${seconds}s`}
      decreaseAriaLabel="Decrease delay"
      increaseAriaLabel="Increase delay"
    />
  );
}

function SceneActionCountdown({ title, secondsLeft, onCancel }) {
  return (
    <div className="scene-drive-panel scene-drive-panel--full scene-drive-panel--countdown">
      <p className="scene-drive-panel__title">{title}</p>
      <div className="scene-drive-countdown">
        <p className="scene-drive-countdown__label">Starting in</p>
        <p className="scene-drive-countdown__value" aria-live="polite">
          {secondsLeft}
        </p>
        <button type="button" className="scene-drive-btn scene-drive-btn--secondary" onClick={onCancel}>
          Cancel
        </button>
      </div>
    </div>
  );
}

/**
 * @param {Object} props
 * @param {string} props.title
 * @param {Record<string, unknown>} props.defaultOptions
 * @param {(options: Record<string, unknown>) => unknown} props.buildPayload
 * @param {(payload: unknown) => void} props.onApply
 * @param {() => void} props.onBack
 * @param {(ctx: { options: Record<string, unknown>, setOption: (key: string, checked: boolean) => void, onChange: (next: Record<string, unknown>) => void }) => import('react').ReactNode} props.renderOptions
 */
export function SceneActionPanel({ title, defaultOptions, buildPayload, onApply, onBack, renderOptions }) {
  const [view, setView] = useState("options");
  const [options, setOptions] = useState(defaultOptions);
  const [secondsLeft, setSecondsLeft] = useState(SCENE_DELAY_DEFAULT_SECONDS);
  const timerRef = useRef(null);
  const panelRef = useRef(null);
  const onBackRef = useRef(onBack);
  const pendingPayloadRef = useRef(buildPayload(defaultOptions));
  const onApplyRef = useRef(onApply);

  onApplyRef.current = onApply;
  onBackRef.current = onBack;

  const clearTimer = () => {
    if (timerRef.current !== null) {
      window.clearInterval(timerRef.current);
      timerRef.current = null;
    }
  };

  useEffect(() => {
    setOptions(defaultOptions);
    setView("options");
    setSecondsLeft(SCENE_DELAY_DEFAULT_SECONDS);
    pendingPayloadRef.current = buildPayload(defaultOptions);
    clearTimer();
  }, [defaultOptions, buildPayload]);

  useEffect(() => () => clearTimer(), []);

  useEffect(() => {
    const handlePointerDown = (event) => {
      if (panelRef.current?.contains(event.target)) return;
      clearTimer();
      onBackRef.current();
    };

    document.addEventListener("mousedown", handlePointerDown);
    document.addEventListener("touchstart", handlePointerDown);

    return () => {
      document.removeEventListener("mousedown", handlePointerDown);
      document.removeEventListener("touchstart", handlePointerDown);
    };
  }, []);

  useEffect(() => {
    if (view !== "countdown") return undefined;

    timerRef.current = window.setInterval(() => {
      setSecondsLeft((prev) => Math.max(0, prev - 1));
    }, 1000);

    return clearTimer;
  }, [view]);

  useEffect(() => {
    if (view !== "countdown" || secondsLeft > 0) return;

    clearTimer();
    onApplyRef.current(pendingPayloadRef.current);
    onBack();
  }, [view, secondsLeft, onBack]);

  const setOption = (key, checked) => setOptions((prev) => ({ ...prev, [key]: checked }));

  const handleConfirm = () => {
    const payload = buildPayload(options);
    if (options.delay) {
      pendingPayloadRef.current = payload;
      setSecondsLeft(options.delaySeconds);
      setView("countdown");
      return;
    }
    onApply(payload);
    onBack();
  };

  const handleCancelCountdown = () => {
    clearTimer();
    setSecondsLeft(options.delaySeconds);
    setView("options");
  };

  return (
    <div className="scenes-modal scenes-modal--detail">
      <div ref={panelRef} className="scenes-modal__panel">
        {view === "countdown" ? (
          <SceneActionCountdown title={title} secondsLeft={secondsLeft} onCancel={handleCancelCountdown} />
        ) : (
          <div className="scene-drive-panel scene-drive-panel--full">
            <p className="scene-drive-panel__title">{title}</p>
            <div className="scene-drive-options scene-drive-options--grid">
              {renderOptions({ options, setOption, onChange: setOptions })}
            </div>
            <button type="button" className="scene-drive-btn scene-drive-btn--primary" onClick={handleConfirm}>
              OK
            </button>
          </div>
        )}
      </div>
    </div>
  );
}
