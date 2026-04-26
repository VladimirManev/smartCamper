import { useState, useEffect } from "react";

const TABLET_LANDSCAPE_QUERY = "(min-width: 900px) and (orientation: landscape)";

/**
 * True for fullscreen 7" style displays (e.g. 1024×600) and larger tablets in landscape.
 * Phone portrait and narrow layouts stay on the classic single-column UI.
 */
export function useTabletLandscape() {
  const [matches, setMatches] = useState(() => {
    if (typeof window === "undefined") return false;
    return window.matchMedia(TABLET_LANDSCAPE_QUERY).matches;
  });

  useEffect(() => {
    const mq = window.matchMedia(TABLET_LANDSCAPE_QUERY);
    const onChange = () => setMatches(mq.matches);
    onChange();
    mq.addEventListener("change", onChange);
    return () => mq.removeEventListener("change", onChange);
  }, []);

  return matches;
}
