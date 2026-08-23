/**
 * Full-screen black overlay when tablet display backlight is off.
 * Captures touch anywhere to wake without activating UI below.
 */

import { useRef } from "react";
import { installClickShield } from "../utils/blockClickThrough";

/**
 * @param {Object} props
 * @param {boolean} props.isVisible
 * @param {Function} props.onWake
 */
export function DisplayWakeOverlay({ isVisible, onWake }) {
  const wakeLockRef = useRef(false);
  const shieldCleanupRef = useRef(null);
  const pointerStartedOnOverlayRef = useRef(false);

  if (!isVisible) {
    wakeLockRef.current = false;
    pointerStartedOnOverlayRef.current = false;
    return null;
  }

  const blockContextMenu = (event) => {
    event.preventDefault();
  };

  const handlePointerDown = (event) => {
    event.preventDefault();
    event.stopPropagation();
    pointerStartedOnOverlayRef.current = true;
    if (event.currentTarget.setPointerCapture) {
      event.currentTarget.setPointerCapture(event.pointerId);
    }
  };

  const handlePointerUp = (event) => {
    event.preventDefault();
    event.stopPropagation();

    if (!pointerStartedOnOverlayRef.current) {
      return;
    }
    pointerStartedOnOverlayRef.current = false;

    if (wakeLockRef.current) {
      return;
    }
    wakeLockRef.current = true;

    if (
      event.currentTarget.releasePointerCapture &&
      event.currentTarget.hasPointerCapture?.(event.pointerId)
    ) {
      event.currentTarget.releasePointerCapture(event.pointerId);
    }

    shieldCleanupRef.current?.();
    shieldCleanupRef.current = installClickShield(500);
    onWake();
  };

  const handlePointerCancel = (event) => {
    event.preventDefault();
    pointerStartedOnOverlayRef.current = false;
    if (
      event.currentTarget.releasePointerCapture &&
      event.currentTarget.hasPointerCapture?.(event.pointerId)
    ) {
      event.currentTarget.releasePointerCapture(event.pointerId);
    }
  };

  return (
    <div
      className="display-wake-overlay"
      role="button"
      aria-label="Tap to wake display"
      onPointerDown={handlePointerDown}
      onPointerUp={handlePointerUp}
      onPointerCancel={handlePointerCancel}
      onContextMenu={blockContextMenu}
    />
  );
}
