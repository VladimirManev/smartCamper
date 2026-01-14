/**
 * Color Themes Configuration
 * Manages color themes for the application
 */

export const themes = {
  "Dark Blue": {
    name: "Dark Blue",
    colors: {
      // Background colors
      bgPrimary: "#0f172a",
      bgSecondary: "#1e293b",
      
      // Accent colors
      accentBlue: "#3b82f6",
      accentBlueDark: "#2563eb",
      accentRed: "#ef4444",
      accentGray: "#94a3b8",
      
      // Text colors
      textPrimary: "#f5f5f5",
      textSecondary: "#94a3b8",
      
      // Transparent variations
      bgPrimary85: "rgba(15, 23, 42, 0.85)",
      bgPrimary95: "rgba(15, 23, 42, 0.95)",
      bgSecondary50: "rgba(30, 41, 59, 0.5)",
      bgSecondary80: "rgba(30, 41, 59, 0.8)",
      
      accentBlue10: "rgba(59, 130, 246, 0.1)",
      accentBlue15: "rgba(59, 130, 246, 0.15)",
      accentBlue20: "rgba(59, 130, 246, 0.2)",
      accentBlue30: "rgba(59, 130, 246, 0.3)",
      accentBlue40: "rgba(59, 130, 246, 0.4)",
      accentBlue50: "rgba(59, 130, 246, 0.5)",
      accentBlue60: "rgba(59, 130, 246, 0.6)",
      accentBlue70: "rgba(59, 130, 246, 0.7)",
      accentBlue80: "rgba(59, 130, 246, 0.8)",
      accentBlue85: "rgba(59, 130, 246, 0.85)",
      accentBlue90: "rgba(59, 130, 246, 0.9)",
      accentBlue95: "rgba(59, 130, 246, 0.95)",
      
      accentRed30: "rgba(239, 68, 68, 0.3)",
      accentRed40: "rgba(239, 68, 68, 0.4)",
      accentRed50: "rgba(239, 68, 68, 0.5)",
      accentRed60: "rgba(239, 68, 68, 0.6)",
      accentRed80: "rgba(239, 68, 68, 0.8)",
      
      accentGray30: "rgba(148, 163, 184, 0.3)",
      
      black20: "rgba(0, 0, 0, 0.2)",
      black30: "rgba(0, 0, 0, 0.3)",
      black50: "rgba(0, 0, 0, 0.5)",
      
      white3: "rgba(255, 255, 255, 0.03)",
      white5: "rgba(255, 255, 255, 0.05)",
      white10: "rgba(255, 255, 255, 0.1)",
    },
  },
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
};

/**
 * Get available theme names
 * @returns {string[]} Array of theme names
 */
export const getThemeNames = () => {
  return Object.keys(themes);
};
