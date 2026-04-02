/**
 * LED strip / relay detail modal: brightness slider (strips only) + same LEDCard control as dashboard.
 */

import { useCallback, useEffect, useRef, useState } from "react";
import { LEDCard } from "./LEDCard";
import { LEDModalArcDimmer } from "./LEDModalArcDimmer";

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
        debounceRef.current = null;
        onBrightness(value);
      }, BRIGHTNESS_DEBOUNCE_MS);
    },
    [onBrightness]
  );

  const commitBrightness = useCallback(
    (value) => {
      if (debounceRef.current) {
        clearTimeout(debounceRef.current);
        debounceRef.current = null;
      }
      onBrightness(value);
    },
    [onBrightness]
  );

  const sliderValue = Math.min(
    255,
    Math.max(1, (sliderOverride !== null ? sliderOverride : brightness) || 1)
  );

  if (variant === "relay") {
    return (
      <div className="led-strip-modal led-strip-modal--relay">
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
    <div className="led-strip-modal led-strip-modal--strip">
      <LEDModalArcDimmer
        strip={strip}
        stripIndex={stripIndex}
        disabled={disabled}
        displayBrightness={sliderValue}
        onBrightnessDrag={(v) => {
          setSliderOverride(v);
          scheduleBrightness(v);
        }}
        onBrightnessCommit={commitBrightness}
        onToggle={onToggle}
      />
    </div>
  );
}

export function getApplianceModalIcon(cardName) {
  return APPLIANCE_ICON_BY_TITLE[cardName] || "bulb";
}
