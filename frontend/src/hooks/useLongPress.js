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
  const hasStartedRef = useRef(false);

  const start = useCallback(
    (event) => {
      isLongPressRef.current = false;
      hasStartedRef.current = true;
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
    (event, shouldTriggerClick = true) => {
      if (timeoutRef.current) {
        clearTimeout(timeoutRef.current);
      }

      // Only trigger onClick if it wasn't a long press and we started on this element
      if (
        shouldTriggerClick &&
        hasStartedRef.current &&
        !isLongPressRef.current &&
        onClick
      ) {
        onClick(event);
      }

      // Reset flags
      hasStartedRef.current = false;
      setTimeout(() => {
        isLongPressRef.current = false;
      }, 0);
    },
    [onClick]
  );

  return {
    onMouseDown: start,
    onTouchStart: start,
    onMouseUp: (e) => clear(e, true),
    onMouseLeave: (e) => clear(e, false), // Don't trigger click on mouse leave
    onTouchEnd: (e) => clear(e, true),
    onTouchCancel: (e) => clear(e, false), // Don't trigger click on touch cancel
  };
};
