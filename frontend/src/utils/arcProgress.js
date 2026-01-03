/**
 * Arc Progress Utility
 * Calculates arc progress for LED brightness visualization
 */

/**
 * Calculate arc progress for brightness visualization
 * Arc is from 135° to 45° = 270 degrees total
 * @param {number} brightness - Brightness value (0-255)
 * @param {boolean} isOn - Whether the LED is on
 * @returns {number} Arc progress length in pixels
 */
export const getArcProgress = (brightness, isOn) => {
  if (!isOn || brightness === 0) {
    return 0;
  }
  
  // Arc length: 270 degrees = π * radius * (270/180) ≈ 377px (for radius 80)
  const arcLength = Math.PI * 80 * (270 / 180);

  // Progress from 0 to 1 according to brightness (0-255)
  const progress = brightness / 255;

  return progress * arcLength;
};

