/**
 * Get theme color from CSS variable
 * @param {string} variableName - CSS variable name (e.g., '--color-accent-blue')
 * @returns {string} Color value
 */
export const getThemeColor = (variableName) => {
  if (typeof window === 'undefined') {
    return ''; // SSR fallback
  }
  return getComputedStyle(document.documentElement).getPropertyValue(variableName).trim();
};
