/**
 * ClockDateCard Component
 * Plain digital clock (HH:MM), Inter / 48px / semibold
 */

import { useState, useEffect } from "react";

/** Props from parent are ignored; sensors are shown in the row above the clock. */
export const ClockDateCard = () => {
  const [time, setTime] = useState(new Date());

  useEffect(() => {
    const timer = setInterval(() => setTime(new Date()), 1000);
    return () => clearInterval(timer);
  }, []);

  const hours = String(time.getHours()).padStart(2, "0");
  const minutes = String(time.getMinutes()).padStart(2, "0");
  const timeString = `${hours}:${minutes}`;
  const dayName = new Intl.DateTimeFormat("en-US", { weekday: "long" })
    .format(time)
    .toLowerCase();
  const monthName = new Intl.DateTimeFormat("en-US", { month: "long" })
    .format(time)
    .toLowerCase();
  const dateLine = `${time.getDate()} ${monthName}`;

  return (
    <div className="clock-date-card">
      <div className="clock-container">
        <time className="clock-time" dateTime={time.toISOString()}>
          {timeString}
        </time>
      </div>
      <div className="clock-calendar" aria-label="Date">
        <div className="clock-calendar-line">{dayName}</div>
        <div className="clock-calendar-line">{dateLine}</div>
      </div>
    </div>
  );
};
