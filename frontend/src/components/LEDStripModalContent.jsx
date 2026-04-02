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
/** Clear local arc only when MQTT state has caught up (avoids stale updates resetting the UI). */
const BRIGHTNESS_MATCH_EPS = 3;

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
    if (sliderOverride === null) return;
    if (Math.abs(brightness - sliderOverride) <= BRIGHTNESS_MATCH_EPS) {
      setSliderOverride(null);
    }
  }, [brightness, sliderOverride]);

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

  const handleBrightnessCommit = useCallback(
    (value) => {
      if (debounceRef.current) {
        clearTimeout(debounceRef.current);
        debounceRef.current = null;
      }
      setSliderOverride(value);
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
        onBrightnessCommit={handleBrightnessCommit}
        onToggle={onToggle}
      />
    </div>
  );
}

export function getApplianceModalIcon(cardName) {
  return APPLIANCE_ICON_BY_TITLE[cardName] || "bulb";
}
