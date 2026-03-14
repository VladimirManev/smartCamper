/**
 * LevelingGroupCard Component
 * Group card for leveling functionality
 * Opens leveling modal when clicked
 */

import { Card } from "./Card";
import { useEffect, useState } from "react";
import { getThemeColor } from "../utils/getThemeColor";

/**
 * LevelingGroupCard component
 * @param {Object} props - Component props
 * @param {string} props.name - Display name (e.g., "Leveling")
 * @param {Function} props.onClick - Click handler function (opens modal)
 * @param {boolean} props.disabled - Whether the control is disabled/offline
 */
export const LevelingGroupCard = ({ name, onClick, disabled = false }) => {
  // Get theme colors for gradients only
  const [accentBlue, setAccentBlue] = useState("#3b82f6");
  const [accentBlueDark, setAccentBlueDark] = useState("#2563eb");
  
  useEffect(() => {
    const updateColors = () => {
      setAccentBlue(getThemeColor("--color-accent-blue"));
      setAccentBlueDark(getThemeColor("--color-accent-blue-dark"));
    };
    
    updateColors();
    const interval = setInterval(updateColors, 2000);
    return () => clearInterval(interval);
  }, []);

  // Generate unique gradient ID based on name
  const gradientId = `gradient-${name.toLowerCase().replace(/\s+/g, "-")}-leveling-group`;

  // Leveling icon (from leveling.svg - converted to use currentColor)
  // Added small arrows under the two circles (wheels) pointing up
  const levelingIcon = (
    <svg viewBox="0 0 239 239" fill="none" xmlns="http://www.w3.org/2000/svg">
      {/* Main vehicle body */}
      <path 
        d="M235.91,117.885l-26.319-19.132l-36.961-45.007c-1.424-1.734-3.551-2.74-5.796-2.74H7.5
        c-4.142,0-7.5,3.357-7.5,7.5v106.779c0,4.143,3.358,7.5,7.5,7.5h19.736c3.105,8.846,11.536,15.209,21.43,15.209
        s18.325-6.363,21.43-15.209h96.141c3.105,8.846,11.536,15.209,21.43,15.209c9.894,0,18.325-6.363,21.43-15.209H231.5
        c4.142,0,7.5-3.357,7.5-7.5v-41.334C239,121.551,237.851,119.296,235.91,117.885z M48.666,172.994
        c-4.251,0-7.709-3.458-7.709-7.709s3.458-7.709,7.709-7.709s7.709,3.458,7.709,7.709S52.917,172.994,48.666,172.994z
         M187.667,172.994c-4.251,0-7.709-3.458-7.709-7.709s3.458-7.709,7.709-7.709c4.25,0,7.708,3.458,7.708,7.709
        S191.918,172.994,187.667,172.994z M224,157.785h-14.896c-3.104-8.848-11.541-15.209-21.436-15.209
        c-9.895,0-18.332,6.361-21.437,15.209H70.103c-3.104-8.848-11.542-15.209-21.437-15.209s-18.332,6.361-21.437,15.209H15V66.006
        h148.288l35.321,43.009c0.405,0.493,0.87,0.932,1.386,1.307L224,127.771V157.785z" 
        fill="currentColor"
        stroke="none"
      />
      {/* Windshield/roof detail */}
      <path 
        d="M162.469,75.551c-2.01-2.448-5.341-3.367-8.324-2.302c-2.982,1.068-4.972,3.894-4.972,7.062v25.766
        c0,4.143,3.358,7.5,7.5,7.5h21.16c0.007,0,0.013,0,0.02,0c4.142,0,7.5-3.357,7.5-7.5c0-1.98-0.768-3.781-2.021-5.122
        L162.469,75.551z" 
        fill="currentColor"
        stroke="none"
      />
      {/* Small arrows under the two wheels pointing up */}
      {/* Arrow under left wheel (centered around x=48.666, below at y~180) */}
      <path 
        d="M 48.666 185 L 46 175 L 48.666 180 L 51.332 175 Z" 
        fill="currentColor"
        stroke="none"
      />
      {/* Arrow under right wheel (centered around x=187.667, below at y~180) */}
      <path 
        d="M 187.667 185 L 185 175 L 187.667 180 L 190.333 175 Z" 
        fill="currentColor"
        stroke="none"
      />
    </svg>
  );

  // Empty horseshoe progress for structure consistency
  const horseshoeProgress = (
    <svg className="horseshoe-progress" viewBox="0 0 200 200">
      <defs>
        <linearGradient id={gradientId} x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stopColor={accentBlue} />
          <stop offset="100%" stopColor={accentBlueDark} />
        </linearGradient>
      </defs>
    </svg>
  );

  return (
    <Card
      name={name}
      icon={levelingIcon}
      buttonState="off"
      iconState="active"
      onClick={onClick}
      disabled={disabled}
    >
      {horseshoeProgress}
    </Card>
  );
};
