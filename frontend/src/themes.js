/**
 * Color Themes Configuration
 * Manages color themes for the application
 */

/** Keys in `themes` — day/night appearance */
export const THEME_DAY_KEY = "Light Gray";
export const THEME_NIGHT_KEY = "Midnight Glass";

export const themes = {
  "Light Gray": {
    name: "Light Gray",
    colors: {
      // Background colors - very light gray instead of white
      bgPrimary: "#f1f5f9",
      bgSecondary: "#f8fafc",
      
      // Accent colors - dark gray instead of blue for better contrast in light theme
      accentBlue: "#475569",
      accentBlueDark: "#334155",
      accentRed: "#ef4444",
      accentGray: "#6b7280",
      
      // Icon color - same as accent blue for consistency
      iconBlue: "#475569",
      
      // Text colors - dark gray/black
      textPrimary: "#1f2937",
      textSecondary: "#6b7280",
      
      // Transparent variations - adjusted for light theme
      bgPrimary85: "rgba(241, 245, 249, 0.85)",
      bgPrimary95: "rgba(241, 245, 249, 0.95)",
      bgSecondary50: "rgba(248, 250, 252, 0.5)",
      bgSecondary80: "rgba(248, 250, 252, 0.8)",
      
      accentBlue10: "rgba(71, 85, 105, 0.1)",
      accentBlue15: "rgba(71, 85, 105, 0.15)",
      accentBlue20: "rgba(71, 85, 105, 0.2)",
      accentBlue30: "rgba(71, 85, 105, 0.3)",
      accentBlue40: "rgba(71, 85, 105, 0.4)",
      accentBlue50: "rgba(71, 85, 105, 0.5)",
      accentBlue60: "rgba(71, 85, 105, 0.6)",
      accentBlue70: "rgba(71, 85, 105, 0.7)",
      accentBlue80: "rgba(71, 85, 105, 0.8)",
      accentBlue85: "rgba(71, 85, 105, 0.85)",
      accentBlue90: "rgba(71, 85, 105, 0.9)",
      accentBlue95: "rgba(71, 85, 105, 0.95)",
      
      // Icon blue variations (same as accent blue for Light Gray theme)
      iconBlue10: "rgba(71, 85, 105, 0.1)",
      iconBlue20: "rgba(71, 85, 105, 0.2)",
      iconBlue30: "rgba(71, 85, 105, 0.3)",
      iconBlue40: "rgba(71, 85, 105, 0.4)",
      iconBlue50: "rgba(71, 85, 105, 0.5)",
      iconBlue60: "rgba(71, 85, 105, 0.6)",
      iconBlue70: "rgba(71, 85, 105, 0.7)",
      iconBlue80: "rgba(71, 85, 105, 0.8)",
      
      accentRed30: "rgba(239, 68, 68, 0.3)",
      accentRed40: "rgba(239, 68, 68, 0.4)",
      accentRed50: "rgba(239, 68, 68, 0.5)",
      accentRed60: "rgba(239, 68, 68, 0.6)",
      accentRed80: "rgba(239, 68, 68, 0.8)",
      
      accentGray30: "rgba(107, 114, 128, 0.3)",
      
      black20: "rgba(0, 0, 0, 0.2)",
      black30: "rgba(0, 0, 0, 0.3)",
      black50: "rgba(0, 0, 0, 0.5)",
      
      white3: "rgba(255, 255, 255, 0.03)",
      white5: "rgba(255, 255, 255, 0.05)",
      white10: "rgba(255, 255, 255, 0.1)",
    },
  },
  "Midnight Glass": {
    name: "Midnight Glass",
    colors: {
      // Background colors - matched to darkest pines on right side of night hero image
      bgPrimary: "#000106",
      bgSecondary: "#1C1F26", // Dark glass/metal surface
      
      // Accent colors - refined neon blue, less saturated
      accentBlue: "#6BA3FF", // Softer neon blue for active elements
      accentBlueDark: "#5B9FFF", // Slightly darker variant
      accentRed: "#ef4444",
      accentGray: "#94a3b8",
      
      // Icon color - same as water color for consistency
      iconBlue: "#6BA3FF",
      
      // Text colors - brighter white for better readability
      textPrimary: "#FFFFFF",
      textSecondary: "#B8C5D6", // Lighter gray for secondary text
      
      // Transparent variations
      bgPrimary85: "rgba(14, 16, 21, 0.85)",
      bgPrimary95: "rgba(14, 16, 21, 0.95)",
      bgSecondary50: "rgba(28, 31, 38, 0.5)",
      bgSecondary80: "rgba(28, 31, 38, 0.8)",
      
      // Softer blue variations for refined glow effects
      accentBlue10: "rgba(107, 163, 255, 0.1)",
      accentBlue15: "rgba(107, 163, 255, 0.15)",
      accentBlue20: "rgba(107, 163, 255, 0.2)",
      accentBlue30: "rgba(107, 163, 255, 0.3)",
      accentBlue40: "rgba(107, 163, 255, 0.4)",
      accentBlue50: "rgba(107, 163, 255, 0.5)",
      accentBlue60: "rgba(107, 163, 255, 0.6)",
      accentBlue70: "rgba(107, 163, 255, 0.7)",
      accentBlue80: "rgba(107, 163, 255, 0.8)",
      accentBlue85: "rgba(107, 163, 255, 0.85)",
      accentBlue90: "rgba(107, 163, 255, 0.9)",
      accentBlue95: "rgba(107, 163, 255, 0.95)",
      
      // Icon blue variations - same as accent blue for consistency with water
      iconBlue10: "rgba(107, 163, 255, 0.1)",
      iconBlue20: "rgba(107, 163, 255, 0.2)",
      iconBlue30: "rgba(107, 163, 255, 0.3)",
      iconBlue40: "rgba(107, 163, 255, 0.4)",
      iconBlue50: "rgba(107, 163, 255, 0.5)",
      iconBlue60: "rgba(107, 163, 255, 0.6)",
      iconBlue70: "rgba(107, 163, 255, 0.7)",
      iconBlue80: "rgba(107, 163, 255, 0.8)",
      
      accentRed30: "rgba(239, 68, 68, 0.3)",
      accentRed40: "rgba(239, 68, 68, 0.4)",
      accentRed50: "rgba(239, 68, 68, 0.5)",
      accentRed60: "rgba(239, 68, 68, 0.6)",
      accentRed80: "rgba(239, 68, 68, 0.8)",
      
      accentGray30: "rgba(148, 163, 184, 0.3)",
      
      // Softer shadows for inward effect
      black20: "rgba(0, 0, 0, 0.2)",
      black30: "rgba(0, 0, 0, 0.3)",
      black50: "rgba(0, 0, 0, 0.5)",
      
      // Softer white highlights for inward shadows
      white3: "rgba(255, 255, 255, 0.03)",
      white5: "rgba(255, 255, 255, 0.05)",
      white10: "rgba(255, 255, 255, 0.1)",
      
      // Shadow colors for inward effect - softer, more blurred
      shadowInsetTop: "rgba(20, 25, 35, 0.4)", // Dark blue-gray for top/left inset
      shadowInsetBottom: "rgba(0, 0, 0, 0.6)", // Darker for bottom/right inset
    },
  },
};

/**
 * Apply theme to the document
 * @param {string} themeName - Name of the theme to apply
 */
export const applyTheme = (themeName) => {
  const theme = themes[themeName];
  if (!theme) {
    console.warn(`Theme "${themeName}" not found, using default`);
    return;
  }

  const root = document.documentElement;
  const { colors } = theme;

  // Apply all color variables
  root.style.setProperty("--color-bg-primary", colors.bgPrimary);
  root.style.setProperty("--color-bg-secondary", colors.bgSecondary);
  root.style.setProperty("--color-accent-blue", colors.accentBlue);
  root.style.setProperty("--color-accent-blue-dark", colors.accentBlueDark);
  root.style.setProperty("--color-accent-red", colors.accentRed);
  root.style.setProperty("--color-accent-gray", colors.accentGray);
  root.style.setProperty("--color-text-primary", colors.textPrimary);
  root.style.setProperty("--color-text-secondary", colors.textSecondary);
  
  // Transparent variations
  root.style.setProperty("--color-bg-primary-85", colors.bgPrimary85);
  root.style.setProperty("--color-bg-primary-95", colors.bgPrimary95);
  root.style.setProperty("--color-bg-secondary-50", colors.bgSecondary50);
  root.style.setProperty("--color-bg-secondary-80", colors.bgSecondary80);
  
  root.style.setProperty("--color-accent-blue-10", colors.accentBlue10);
  root.style.setProperty("--color-accent-blue-15", colors.accentBlue15);
  root.style.setProperty("--color-accent-blue-20", colors.accentBlue20);
  root.style.setProperty("--color-accent-blue-30", colors.accentBlue30);
  root.style.setProperty("--color-accent-blue-40", colors.accentBlue40);
  root.style.setProperty("--color-accent-blue-50", colors.accentBlue50);
  root.style.setProperty("--color-accent-blue-60", colors.accentBlue60);
  root.style.setProperty("--color-accent-blue-70", colors.accentBlue70);
  root.style.setProperty("--color-accent-blue-80", colors.accentBlue80);
  root.style.setProperty("--color-accent-blue-85", colors.accentBlue85);
  root.style.setProperty("--color-accent-blue-90", colors.accentBlue90);
  root.style.setProperty("--color-accent-blue-95", colors.accentBlue95);
  
  root.style.setProperty("--color-accent-red-30", colors.accentRed30);
  root.style.setProperty("--color-accent-red-40", colors.accentRed40);
  root.style.setProperty("--color-accent-red-50", colors.accentRed50);
  root.style.setProperty("--color-accent-red-60", colors.accentRed60);
  root.style.setProperty("--color-accent-red-80", colors.accentRed80);
  
  root.style.setProperty("--color-accent-gray-30", colors.accentGray30);
  
  root.style.setProperty("--color-black-20", colors.black20);
  root.style.setProperty("--color-black-30", colors.black30);
  root.style.setProperty("--color-black-50", colors.black50);
  
  root.style.setProperty("--color-white-3", colors.white3);
  root.style.setProperty("--color-white-5", colors.white5);
  root.style.setProperty("--color-white-10", colors.white10);
  
  // Icon blue variations - always set (all themes now have iconBlue)
  root.style.setProperty("--color-icon-blue", colors.iconBlue || colors.accentBlue);
  root.style.setProperty("--color-icon-blue-10", colors.iconBlue10 || colors.accentBlue10);
  root.style.setProperty("--color-icon-blue-20", colors.iconBlue20 || colors.accentBlue20);
  root.style.setProperty("--color-icon-blue-30", colors.iconBlue30 || colors.accentBlue30);
  root.style.setProperty("--color-icon-blue-40", colors.iconBlue40 || colors.accentBlue40);
  root.style.setProperty("--color-icon-blue-50", colors.iconBlue50 || colors.accentBlue50);
  root.style.setProperty("--color-icon-blue-60", colors.iconBlue60 || colors.accentBlue60);
  root.style.setProperty("--color-icon-blue-70", colors.iconBlue70 || colors.accentBlue70);
  root.style.setProperty("--color-icon-blue-80", colors.iconBlue80 || colors.accentBlue80);
  
  // Set leveling icon filter based on icon blue color
  const iconColor = (colors.iconBlue || colors.accentBlue).toLowerCase().trim();
  
  // Map specific colors to their optimal filters
  const colorFilterMap = {
    '#6ba3ff': 'brightness(0) saturate(100%) invert(45%) sepia(96%) saturate(2000%) hue-rotate(200deg) brightness(1.05) contrast(1)', // Midnight Glass icon blue (same as water)
    '#5b9fff': 'brightness(0) saturate(100%) invert(45%) sepia(96%) saturate(2000%) hue-rotate(200deg) brightness(1.05) contrast(1)', // Midnight Glass dark
    '#475569': 'brightness(0) saturate(100%) invert(50%) sepia(20%) saturate(500%) hue-rotate(200deg) brightness(0.8) contrast(1.2)', // Light Gray
  };
  
  // Note: Leveling icon now uses SVG with currentColor, so no filter needed
  // Keeping this for backwards compatibility if needed, but not used anymore
  const filter = colorFilterMap[iconColor] || 'brightness(0) saturate(100%) invert(45%) sepia(96%) saturate(2000%) hue-rotate(210deg) brightness(1) contrast(1)';
  root.style.setProperty("--leveling-icon-filter", filter);
  
  // Shadow colors for inward effect (if available)
  if (colors.shadowInsetTop) {
    root.style.setProperty("--color-shadow-inset-top", colors.shadowInsetTop);
    root.style.setProperty("--color-shadow-inset-bottom", colors.shadowInsetBottom);
  }
};

/**
 * Get available theme names
 * @returns {string[]} Array of theme names
 */
export const getThemeNames = () => {
  return Object.keys(themes);
};
