/**
 * ClockDateCard — digital time (Inter 48px / 600); optional calendar lines below.
 */

import { useState, useEffect } from "react";
import { THEME_NIGHT_KEY } from "../themes";

function formatCalendarLines(date) {
  const dayName = new Intl.DateTimeFormat("en-US", { weekday: "long" })
    .format(date)
    .toLowerCase();
  const monthName = new Intl.DateTimeFormat("en-US", { month: "long" })
    .format(date)
    .toLowerCase();
  const dateLine = `${date.getDate()} ${monthName}`;
  return { dayName, dateLine };
}

/**
 * Day + date lines (Inter 12–15px / 400). Separate from clock so layout can place it outside the temps row.
 * Tablet: appearance chip — toggle when settings Mode is Automatic; status only for Day/Night.
 *
 * @param {Object} props
 * @param {string} props.resolvedTheme - Applied palette key ("Light Gray" | "Midnight Glass")
 * @param {'day'|'night'|'automatic'} props.settingsMode
 * @param {() => void} [props.onAppearanceToggle]
 */
export function ClockCalendarLines({
  resolvedTheme,
  settingsMode,
  onAppearanceToggle,
}) {
  const [time, setTime] = useState(new Date());

  useEffect(() => {
    const tick = () => setTime(new Date());
    tick();
    const id = setInterval(tick, 60_000);
    return () => clearInterval(id);
  }, []);

  const { dayName, dateLine } = formatCalendarLines(time);

  const isNightLook = resolvedTheme === THEME_NIGHT_KEY;
  const chipLabel = isNightLook ? "Night mode" : "Day mode";
  const isToggle = settingsMode === "automatic" && typeof onAppearanceToggle === "function";

  return (
    <div className="clock-calendar-block">
      <div className="clock-calendar" aria-label="Date">
        <div className="clock-calendar-line">{dayName}</div>
        <div className="clock-calendar-line">{dateLine}</div>
      </div>
      {isToggle ? (
        <button
          type="button"
          className="night-mode-chip night-mode-chip--toggle"
          onClick={onAppearanceToggle}
          aria-pressed={isNightLook}
        >
          {chipLabel}
        </button>
      ) : (
        <div className="night-mode-chip" role="status">
          {chipLabel}
        </div>
      )}
    </div>
  );
}

/**
 * @param {boolean} [props.showCalendar=true] — if false, only HH:MM (for tablet cluster next to sensors).
 */
export const ClockDateCard = ({ showCalendar = true }) => {
  const [time, setTime] = useState(new Date());

  useEffect(() => {
    const timer = setInterval(() => setTime(new Date()), 1000);
    return () => clearInterval(timer);
  }, []);

  const hours = String(time.getHours()).padStart(2, "0");
  const minutes = String(time.getMinutes()).padStart(2, "0");
  const timeString = `${hours}:${minutes}`;
  const calendar = showCalendar ? formatCalendarLines(time) : null;

  return (
    <div
      className={`clock-date-card${showCalendar ? "" : " clock-date-card--time-only"}`}
    >
      <div className="clock-container">
        <time className="clock-time" dateTime={time.toISOString()}>
          {timeString}
        </time>
      </div>
      {calendar ? (
        <div className="clock-calendar" aria-label="Date">
          <div className="clock-calendar-line">{calendar.dayName}</div>
          <div className="clock-calendar-line">{calendar.dateLine}</div>
        </div>
      ) : null}
    </div>
  );
};
