/**
 * LED strip / relay detail modal: brightness slider (strips only) + same LEDCard control as dashboard.
 */

import { useCallback, useEffect, useRef, useState } from "react";
import { LEDCard } from "./LEDCard";

const APPLIANCE_ICON_BY_TITLE = {
  Audio: "audio",
  Pump: "pump",
  Fridge: "fridge",
  "WC Fan": "fan",
  Boiler: "boiler",
  Inverter: "bulb",
};

const BRIGHTNESS_DEBOUNCE_MS = 150;

export function LEDStripModalContent({
  variant,
  strip,
  stripIndex,
  onBrightness,
  onToggle,
  disabled,
  icon = "bulb",
}) {
  const debounceRef = useRef(null);
  const [sliderOverride, setSliderOverride] = useState(null);

  useEffect(() => {
    return () => {
      if (debounceRef.current) clearTimeout(debounceRef.current);
    };
  }, []);

  const brightness = strip?.brightness ?? 0;
  useEffect(() => {
    setSliderOverride(null);
  }, [brightness]);

  const scheduleBrightness = useCallback(
    (value) => {
      if (debounceRef.current) clearTimeout(debounceRef.current);
      debounceRef.current = setTimeout(() => {
        onBrightness(value);
      }, BRIGHTNESS_DEBOUNCE_MS);
    },
    [onBrightness]
  );

  const sliderValue = Math.min(
    255,
    Math.max(1, (sliderOverride !== null ? sliderOverride : brightness) || 1)
  );

  if (variant === "relay") {
    return (
      <div className="led-strip-modal">
        <div className="led-strip-modal__card">
          <LEDCard
            name=""
            strip={strip}
            onClick={onToggle}
            type="relay"
            disabled={disabled}
            icon={icon}
          />
        </div>
      </div>
    );
  }

  return (
    <div className="led-strip-modal">
      <label className="led-strip-modal__brightness-label" htmlFor={`led-brightness-${stripIndex}`}>
        <span>Brightness</span>
        <span>{sliderOverride !== null ? sliderOverride : brightness}</span>
      </label>
      <input
        id={`led-brightness-${stripIndex}`}
        className="led-strip-modal__range"
        type="range"
        min={1}
        max={255}
        value={sliderValue}
        disabled={disabled}
        onChange={(e) => {
          const v = Number(e.target.value);
          setSliderOverride(v);
          scheduleBrightness(v);
        }}
      />
      <div className="led-strip-modal__card">
        <LEDCard
          name=""
          strip={strip}
          onClick={onToggle}
          type="strip"
          disabled={disabled}
          icon="bulb"
        />
      </div>
    </div>
  );
}

export function getApplianceModalIcon(cardName) {
  return APPLIANCE_ICON_BY_TITLE[cardName] || "bulb";
}
