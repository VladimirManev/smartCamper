import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import "./index.css";
import App from "./App.jsx";

// Import Font Awesome CSS locally (works offline)
import "@fortawesome/fontawesome-free/css/all.min.css";

// Проверка за поддръжка на браузъра
function checkBrowserSupport() {
  // Проверка за ES6 модули
  if (typeof Symbol === "undefined") {
    return { supported: false, reason: "ES6 не се поддържа" };
  }
  
  // Проверка за Promise
  if (typeof Promise === "undefined") {
    return { supported: false, reason: "Promise не се поддържа" };
  }
  
  // Проверка за fetch (може да не е налична на стари Safari)
  if (typeof fetch === "undefined") {
    console.warn("Fetch API не се поддържа, но приложението може да работи");
  }
  
  return { supported: true };
}

// Fallback за стари браузъри, които не поддържат createRoot
const rootElement = document.getElementById("root");

if (!rootElement) {
  console.error("Root element not found!");
  document.body.innerHTML = `
    <div style="padding: 20px; color: white; text-align: center; font-family: Arial, sans-serif; background: #0f172a; min-height: 100vh; display: flex; flex-direction: column; justify-content: center; align-items: center;">
      <h2>Грешка</h2>
      <p>Root елементът не е намерен.</p>
    </div>
  `;
} else {
  const browserCheck = checkBrowserSupport();
  
  if (!browserCheck.supported) {
    rootElement.innerHTML = `
      <div style="padding: 20px; color: white; text-align: center; font-family: Arial, sans-serif; background: #0f172a; min-height: 100vh; display: flex; flex-direction: column; justify-content: center; align-items: center;">
        <h2>Браузърът не се поддържа</h2>
        <p>${browserCheck.reason}</p>
        <p style="margin-top: 20px;">Моля, използвайте по-нова версия на Safari или друг модерен браузър.</p>
      </div>
    `;
  } else {
    try {
      // Проверка дали createRoot се поддържа
      if (typeof createRoot === "function") {
        createRoot(rootElement).render(
          <StrictMode>
            <App />
          </StrictMode>
        );
      } else {
        throw new Error("createRoot не се поддържа от този браузър");
      }
    } catch (error) {
      console.error("Error rendering app:", error);
      // Показваме съобщение за грешка на потребителя
      rootElement.innerHTML = `
        <div style="padding: 20px; color: white; text-align: center; font-family: Arial, sans-serif; background: #0f172a; min-height: 100vh; display: flex; flex-direction: column; justify-content: center; align-items: center;">
          <h2>Грешка при зареждане</h2>
          <p>Вашият браузър може да не е съвместим. Моля, опитайте с по-нова версия на Safari или друг браузър.</p>
          <p style="font-size: 12px; color: #999; margin-top: 20px;">Error: ${error.message}</p>
          <p style="font-size: 12px; color: #999;">User Agent: ${navigator.userAgent}</p>
        </div>
      `;
    }
  }
}
