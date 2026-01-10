/**
 * ClockDateCard Component
 * Displays clock (hours:minutes) and date
 * Clock has neumorphic outer circle with blue glow and inner seconds indicator as arc
 */

import { useState, useEffect } from "react";

/**
 * ClockDateCard component
 * @param {Object} props - Component props
 * @param {number} props.indoorTemp - Indoor temperature value
 * @param {number} props.outdoorTemp - Outdoor temperature value
 * @param {number} props.humidity - Indoor humidity value
 */
export const ClockDateCard = ({ indoorTemp, outdoorTemp, humidity }) => {
  const [time, setTime] = useState(new Date());
  const [date, setDate] = useState(new Date());

  // Update time every second
  useEffect(() => {
    const timer = setInterval(() => {
      const now = new Date();
      setTime(now);
      setDate(now);
    }, 1000);

    return () => clearInterval(timer);
  }, []);

  // Format time: HH:MM
  const hours = String(time.getHours()).padStart(2, "0");
  const minutes = String(time.getMinutes()).padStart(2, "0");
  const timeString = `${hours}:${minutes}`;

  // Format date: day + month abbreviation (e.g., "23 DEC")
  const day = date.getDate();
  const monthNames = [
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  ];
  const month = monthNames[date.getMonth()];
  const dateString = `${day} ${month}`;

  // Get day of week name (English)
  const dayNames = ["SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"];
  const dayName = dayNames[date.getDay()];

  // Calculate seconds for arc progress (0-60)
  const seconds = time.getSeconds();
  
  // Arc progress: full circle (360 degrees) for 60 seconds
  // SVG circle circumference = 2 * π * radius
  // SVG viewBox is "0 0 200 200", center is at 100,100
  // Radius = 90 (in viewBox coordinates) - very close to outer edge
  const radius = 90;
  const circumference = 2 * Math.PI * radius;
  const progress = (seconds / 60) * circumference;
  
  // Format temperatures and humidity
  const indoorTempFormatted = indoorTemp !== null && indoorTemp !== undefined 
    ? `${indoorTemp.toFixed(1)}°` 
    : "--°";
  const outdoorTempFormatted = outdoorTemp !== null && outdoorTemp !== undefined 
    ? `${outdoorTemp.toFixed(1)}°` 
    : "--°";
  const humidityFormatted = humidity !== null && humidity !== undefined 
    ? `${humidity.toFixed(0)}%` 
    : "--%";

  return (
    <div className="clock-date-card">
      {/* Clock */}
      <div className="clock-container">
        <div className="neumorphic-button clock-button">
          {/* Inner seconds arc (full circle) */}
          <svg className="clock-seconds-arc" viewBox="0 0 200 200">
            <defs>
              <linearGradient id="seconds-gradient" x1="0%" y1="0%" x2="0%" y2="100%">
                <stop offset="0%" stopColor="#3b82f6" />
                <stop offset="100%" stopColor="#2563eb" />
              </linearGradient>
            </defs>
            {/* Background circle (subtle) */}
            <circle
              cx="100"
              cy="100"
              r={radius}
              fill="none"
              stroke="rgba(59, 130, 246, 0.1)"
              strokeWidth="2"
            />
            {/* Seconds progress arc (full circle, starting from top - 12 o'clock) */}
            {/* Circle path starting from top: we use rotate(-90) to start from 12 o'clock instead of 3 o'clock */}
            <circle
              cx="100"
              cy="100"
              r={radius}
              fill="none"
              stroke="url(#seconds-gradient)"
              strokeWidth="2"
              strokeLinecap="round"
              strokeDasharray={`${progress} ${circumference}`}
              strokeDashoffset={0}
              transform="rotate(-90 100 100)" // Rotate -90deg: starts from top (12 o'clock) instead of right (3 o'clock)
            />
          </svg>
          
          {/* Time display (HH:MM) */}
          <div className="clock-time">
            {timeString}
          </div>
        </div>
      </div>

      {/* Date text below clock */}
      <div className="date-text-container">
        <div className="date-text">
          <span className="day-name">{dayName}</span>
          <span className="date-value">{dateString}</span>
        </div>
      </div>
    </div>
  );
};

