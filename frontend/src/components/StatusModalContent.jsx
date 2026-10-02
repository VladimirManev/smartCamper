/**
 * Tablet Status panel — rotates battery, tanks, fridge, and doors every 4s.
 * Swipe left/right to change slides manually (resets auto-rotate timer).
 */

import { useCallback, useEffect, useRef, useState } from "react";
import { BatteryModalContent } from "./BatteryModalContent";
import { GrayWaterModalContent } from "./GrayWaterModalContent";
import { FreshWaterModalContent } from "./FreshWaterModalContent";
import { ToiletUrineModalContent } from "./ToiletUrineModalContent";
import { FridgeModalContent } from "./FridgeModalContent";
import { CamperDoorsStage, DEFAULT_DOORS } from "./CamperDoorsStage";

const SLIDES = [
  "battery",
  "gray-water",
  "fresh-water",
  "toilet-urine",
  "fridge",
  "doors",
];
const SLIDE_TITLES = [
  "Battery",
  "Gray Water",
  "Fresh Water",
  "Toilet",
  "Fridge",
  "",
];
const ROTATE_MS = 4000;
const SWIPE_MIN_PX = 45;

function wrapIndex(index) {
  const n = SLIDES.length;
  return ((index % n) + n) % n;
}

/**
 * @param {Object} props
 * @param {number|null} props.batteryLevel
 * @param {Object} props.nodes
 * @param {Object} props.wireAmps
 * @param {Object} [props.batteryFlow]
 * @param {number|null} [props.batteryVoltage]
 * @param {number|null} [props.batteryTemperature]
 * @param {Record<string, boolean>} [props.offlineByNode]
 * @param {Record<string, boolean>} [props.offlineByWire]
 * @param {Record<string, string|null>} [props.phaseByNode]
 * @param {boolean} [props.smartShuntOffline]
 * @param {boolean} props.batteryDisabled
 * @param {Function} [props.onOpenBatteryHistory]
 * @param {number|null} props.grayWaterLevel
 * @param {number|null} props.grayWaterTemperature
 * @param {boolean} props.grayWaterDisabled
 * @param {number|null} props.cleanWaterLevel
 * @param {boolean} props.cleanWaterDisabled
 * @param {number|null} props.toiletUrineLevel
 * @param {boolean} props.toiletUrineDisabled
 * @param {Object} [props.fridgeStatus]
 * @param {boolean} [props.fridgePowerOn]
 * @param {boolean} [props.fridgeDisabled]
 * @param {Object} [props.doors] - { driver, passenger, sliding, rear }
 * @param {(title: string) => void} [props.onActiveSlideChange]
 */
export function StatusModalContent({
  batteryLevel,
  nodes,
  wireAmps,
  batteryFlow,
  batteryVoltage,
  batteryTemperature,
  offlineByNode,
  offlineByWire,
  phaseByNode,
  smartShuntOffline,
  batteryDisabled = false,
  onOpenBatteryHistory,
  grayWaterLevel,
  grayWaterTemperature,
  grayWaterDisabled = false,
  cleanWaterLevel,
  cleanWaterDisabled = false,
  toiletUrineLevel,
  toiletUrineDisabled = false,
  fridgeStatus,
  fridgePowerOn = false,
  fridgeDisabled = false,
  doors = DEFAULT_DOORS,
  onActiveSlideChange,
}) {
  const [activeIndex, setActiveIndex] = useState(0);
  const pointerStartRef = useRef(null);
  const suppressClickUntilRef = useRef(0);

  // Auto-advance; restarts whenever the active slide changes (incl. swipe)
  useEffect(() => {
    const timerId = window.setTimeout(() => {
      setActiveIndex((index) => wrapIndex(index + 1));
    }, ROTATE_MS);

    return () => {
      window.clearTimeout(timerId);
    };
  }, [activeIndex]);

  useEffect(() => {
    onActiveSlideChange?.(SLIDE_TITLES[activeIndex]);
  }, [activeIndex, onActiveSlideChange]);

  const goRelative = useCallback((delta) => {
    setActiveIndex((index) => wrapIndex(index + delta));
  }, []);

  const handlePointerDown = (event) => {
    if (event.pointerType === "mouse" && event.button !== 0) {
      return;
    }
    pointerStartRef.current = {
      x: event.clientX,
      y: event.clientY,
      id: event.pointerId,
    };
  };

  const handlePointerUp = (event) => {
    const start = pointerStartRef.current;
    pointerStartRef.current = null;
    if (!start || start.id !== event.pointerId) {
      return;
    }
    const dx = event.clientX - start.x;
    const dy = event.clientY - start.y;
    if (Math.abs(dx) < SWIPE_MIN_PX || Math.abs(dx) < Math.abs(dy)) {
      return;
    }
    suppressClickUntilRef.current = Date.now() + 450;
    // Swipe left → next; swipe right → previous
    goRelative(dx < 0 ? 1 : -1);
  };

  const handlePointerCancel = () => {
    pointerStartRef.current = null;
  };

  const handleClickCapture = (event) => {
    if (Date.now() < suppressClickUntilRef.current) {
      event.preventDefault();
      event.stopPropagation();
    }
  };

  const paneClass = (index) =>
    `status-modal__pane${index === activeIndex ? " status-modal__pane--active" : ""}`;

  return (
    <div className="status-modal">
      <div
        className="status-modal__viewport"
        onPointerDown={handlePointerDown}
        onPointerUp={handlePointerUp}
        onPointerCancel={handlePointerCancel}
        onClickCapture={handleClickCapture}
      >
        <div className={paneClass(0)} aria-hidden={activeIndex !== 0}>
          <BatteryModalContent
            variant="status"
            batteryLevel={batteryLevel}
            nodes={nodes}
            wireAmps={wireAmps}
            batteryFlow={batteryFlow}
            batteryVoltage={batteryVoltage}
            batteryTemperature={batteryTemperature}
            offlineByNode={offlineByNode}
            offlineByWire={offlineByWire}
            phaseByNode={phaseByNode}
            smartShuntOffline={smartShuntOffline}
            disabled={batteryDisabled}
            onOpenHistory={onOpenBatteryHistory}
          />
        </div>
        <div className={paneClass(1)} aria-hidden={activeIndex !== 1}>
          <GrayWaterModalContent
            level={grayWaterLevel}
            temperature={grayWaterTemperature}
            disabled={grayWaterDisabled}
          />
        </div>
        <div className={paneClass(2)} aria-hidden={activeIndex !== 2}>
          <FreshWaterModalContent
            level={cleanWaterLevel}
            disabled={cleanWaterDisabled}
          />
        </div>
        <div className={paneClass(3)} aria-hidden={activeIndex !== 3}>
          <ToiletUrineModalContent
            level={toiletUrineLevel}
            disabled={toiletUrineDisabled}
          />
        </div>
        <div
          className={paneClass(4)}
          aria-hidden={activeIndex !== 4}
          aria-label="Fridge"
        >
          <FridgeModalContent
            status={fridgeStatus}
            powerOn={fridgePowerOn}
            disabled={fridgeDisabled}
            readOnly
          />
        </div>
        <div
          className={paneClass(5)}
          aria-hidden={activeIndex !== 5}
          aria-label="Camper doors"
        >
          <div className="status-modal__doors">
            <CamperDoorsStage
              doors={doors}
              stageClassName="status-modal__doors-stage"
              vanLayerClassName="status-modal__doors-van-layer"
            />
          </div>
        </div>
      </div>
      <div className="status-modal__dots" role="tablist" aria-label="Status slides">
        {SLIDES.map((slideId, index) => (
          <button
            key={slideId}
            type="button"
            className={`status-modal__dot${
              index === activeIndex ? " status-modal__dot--active" : ""
            }`}
            role="tab"
            aria-selected={index === activeIndex}
            aria-label={`Slide ${index + 1} of ${SLIDES.length}`}
            onClick={() => setActiveIndex(index)}
          />
        ))}
      </div>
    </div>
  );
}
