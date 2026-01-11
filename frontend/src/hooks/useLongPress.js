import { useRef, useCallback } from "react";

/**
 * useLongPress hook
 * Detects long press (hold) events for both touch and mouse
 * Prevents onClick from firing if long press is detected
 * @param {Function} onLongPress - Callback when long press is detected
 * @param {Function} onClick - Click handler (only fires if not long press)
 * @param {number} delay - Delay in ms before long press is triggered (default: 500)
 * @returns {Object} Event handlers to attach to element
 */
export const useLongPress = (onLongPress, onClick, delay = 500) => {
  const timeoutRef = useRef(null);
  const isLongPressRef = useRef(false);

  const start = useCallback(
    (event) => {
      isLongPressRef.current = false;
      timeoutRef.current = setTimeout(() => {
        isLongPressRef.current = true;
        if (onLongPress) {
          onLongPress(event);
        }
      }, delay);
    },
    [onLongPress, delay]
  );

  const clear = useCallback(
    (event) => {
      if (timeoutRef.current) {
        clearTimeout(timeoutRef.current);
      }

      // Only trigger onClick if it wasn't a long press
      if (!isLongPressRef.current && onClick) {
        onClick(event);
      }

      // Reset after a short delay to allow onClick to fire
      setTimeout(() => {
        isLongPressRef.current = false;
      }, 0);
    },
    [onClick]
  );

  return {
    onMouseDown: start,
    onTouchStart: start,
    onMouseUp: clear,
    onMouseLeave: clear,
    onTouchEnd: clear,
  };
};
