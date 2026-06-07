/**
 * Tablet Status panel — rotates battery, water tanks, and overview every 4s.
 */

import { useEffect, useRef, useState } from "react";
import { BatteryModalContent } from "./BatteryModalContent";
import { GrayWaterModalContent } from "./GrayWaterModalContent";
import { FreshWaterModalContent } from "./FreshWaterModalContent";
import { ToiletUrineModalContent } from "./ToiletUrineModalContent";
import { ClockDateCard } from "./ClockDateCard";

const SLIDES = [
  "battery",
  "gray-water",
  "fresh-water",
  "toilet-urine",
  "overview",
];
const SLIDE_TITLES = ["Battery", "Gray Water", "Fresh Water", "Toilet", ""];
const ROTATE_MS = 4000;

function formatSensorValue(value, suffix, decimals = 1) {
  if (value === null || value === undefined) return "—";
  return `${Number(value).toFixed(decimals)}${suffix}`;
}

/**
 * @param {Object} props
 * @param {number|null} props.batteryLevel
 * @param {Object} props.nodes
 * @param {Object} props.wireAmps
 * @param {Object} [props.batteryFlow]
 * @param {Record<string, boolean>} [props.offlineByNode]
 * @param {Record<string, boolean>} [props.offlineByWire]
 * @param {boolean} [props.smartShuntOffline]
 * @param {boolean} props.batteryDisabled
 * @param {number|null} props.grayWaterLevel
 * @param {number|null} props.grayWaterTemperature
 * @param {boolean} props.grayWaterDisabled
 * @param {number|null} props.cleanWaterLevel
 * @param {boolean} props.cleanWaterDisabled
 * @param {number|null} props.toiletUrineLevel
 * @param {boolean} props.toiletUrineDisabled
 * @param {number|null} props.indoorTemperature
 * @param {number|null} props.indoorHumidity
 * @param {number|null} props.outdoorTemperature
 * @param {boolean} props.sensorsDisabled
 * @param {(title: string) => void} [props.onActiveSlideChange]
 */
export function StatusModalContent({
  batteryLevel,
  nodes,
  wireAmps,
  batteryFlow,
  offlineByNode,
  offlineByWire,
  smartShuntOffline,
  batteryDisabled = false,
  grayWaterLevel,
  grayWaterTemperature,
  grayWaterDisabled = false,
  cleanWaterLevel,
  cleanWaterDisabled = false,
  toiletUrineLevel,
  toiletUrineDisabled = false,
  indoorTemperature,
  indoorHumidity,
  outdoorTemperature,
  sensorsDisabled = false,
  onActiveSlideChange,
}) {
  const [activeIndex, setActiveIndex] = useState(0);
  const rotateTimerRef = useRef(null);

  useEffect(() => {
    const scheduleNext = () => {
      rotateTimerRef.current = window.setTimeout(() => {
        setActiveIndex((index) => (index + 1) % SLIDES.length);
        scheduleNext();
      }, ROTATE_MS);
    };

    scheduleNext();

    return () => {
      if (rotateTimerRef.current !== null) {
        window.clearTimeout(rotateTimerRef.current);
        rotateTimerRef.current = null;
      }
    };
  }, []);

  useEffect(() => {
    onActiveSlideChange?.(SLIDE_TITLES[activeIndex]);
  }, [activeIndex, onActiveSlideChange]);

  const paneClass = (index) =>
    `status-modal__pane${index === activeIndex ? " status-modal__pane--active" : ""}`;

  return (
    <div className="status-modal">
      <div className="status-modal__viewport">
        <div className={paneClass(0)} aria-hidden={activeIndex !== 0}>
          <BatteryModalContent
            batteryLevel={batteryLevel}
            nodes={nodes}
            wireAmps={wireAmps}
            batteryFlow={batteryFlow}
            offlineByNode={offlineByNode}
            offlineByWire={offlineByWire}
            smartShuntOffline={smartShuntOffline}
            disabled={batteryDisabled}
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
          aria-label="Time, date, and climate"
        >
          <div className="status-modal__overview">
            <ClockDateCard showCalendar />
            <div className="status-modal__sensors status-modal__sensors--inline">
              <div className="status-modal__sensor-item">
                <i className="fas fa-thermometer-half status-modal__sensor-icon" aria-hidden />
                <div className="status-modal__sensor-content">
                  <span className="status-modal__sensor-label">IN</span>
                  <span className="status-modal__sensor-value">
                    {sensorsDisabled
                      ? "—"
                      : formatSensorValue(indoorTemperature, "°", 1)}
                  </span>
                </div>
              </div>
              <div className="status-modal__sensor-item">
                <i className="fas fa-tint status-modal__sensor-icon" aria-hidden />
                <div className="status-modal__sensor-content">
                  <span className="status-modal__sensor-label">IN</span>
                  <span className="status-modal__sensor-value">
                    {sensorsDisabled
                      ? "—"
                      : formatSensorValue(indoorHumidity, "%", 0)}
                  </span>
                </div>
              </div>
              <div className="status-modal__sensor-item">
                <i className="fas fa-thermometer-half status-modal__sensor-icon" aria-hidden />
                <div className="status-modal__sensor-content">
                  <span className="status-modal__sensor-label">OUT</span>
                  <span className="status-modal__sensor-value">
                    {sensorsDisabled
                      ? "—"
                      : formatSensorValue(outdoorTemperature, "°", 1)}
                  </span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div className="status-modal__dots" role="tablist" aria-label="Status slides">
        {SLIDES.map((slideId, index) => (
          <span
            key={slideId}
            className={`status-modal__dot${
              index === activeIndex ? " status-modal__dot--active" : ""
            }`}
            role="tab"
            aria-selected={index === activeIndex}
            aria-label={`Slide ${index + 1} of ${SLIDES.length}`}
          />
        ))}
      </div>
    </div>
  );
}
