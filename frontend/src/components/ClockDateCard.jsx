/**
 * ClockDateCard — digital time (Inter 48px / 600); optional calendar lines below.
 */

import { useState, useEffect } from "react";
import { useLongPress } from "../hooks/useLongPress";

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
 */
export function ClockCalendarLines() {
  const [time, setTime] = useState(new Date());

  useEffect(() => {
    const tick = () => setTime(new Date());
    tick();
    const id = setInterval(tick, 60_000);
    return () => clearInterval(id);
  }, []);

  const { dayName, dateLine } = formatCalendarLines(time);

  return (
    <div className="clock-calendar-block">
      <div className="clock-calendar" aria-label="Date">
        <div className="clock-calendar-line">{dayName}</div>
        <div className="clock-calendar-line">{dateLine}</div>
      </div>
    </div>
  );
}

/**
 * @param {boolean} [props.showCalendar=true] — if false, only HH:MM (for tablet cluster next to sensors).
 * @param {Function} [props.onLongPress] — long-press handler (tablet: turn display backlight off).
 */
export const ClockDateCard = ({ showCalendar = true, onLongPress }) => {
  const longPressHandlers = useLongPress(onLongPress, undefined, 500);

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
      className={`clock-date-card${showCalendar ? "" : " clock-date-card--time-only"}${onLongPress ? " clock-date-card--long-press" : ""}`}
      {...(onLongPress ? longPressHandlers : {})}
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
