import { useState, useEffect } from "react";

const PORTRAIT_QUERY = "(orientation: portrait)";

/**
 * True when the viewport is in portrait orientation (CSS media query).
 */
export function usePortrait() {
  const [matches, setMatches] = useState(() => {
    if (typeof window === "undefined") return true;
    return window.matchMedia(PORTRAIT_QUERY).matches;
  });

  useEffect(() => {
    const mq = window.matchMedia(PORTRAIT_QUERY);
    const onChange = () => setMatches(mq.matches);
    onChange();
    mq.addEventListener("change", onChange);
    return () => mq.removeEventListener("change", onChange);
  }, []);

  return matches;
}
