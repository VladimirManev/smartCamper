// Static Files Middleware
// Сервира статичните файлове от React build и пренасочва всички routes към index.html

const path = require("path");
const express = require("express");

// Път към build директорията на React приложението
const frontendBuildPath = path.join(__dirname, "../../frontend/dist");

// Middleware за сервиране на статични файлове (JS, CSS, images, etc.)
const staticMiddleware = express.static(frontendBuildPath, {
  // Cache headers за статични файлове (1 година)
  maxAge: "1y",
  // ETag за оптимизация
  etag: true,
  // Last-Modified header
  lastModified: true,
});

// Fallback middleware - пренасочва всички routes към index.html
// Това е необходимо за React Router (client-side routing)
const fallbackMiddleware = (req, res, next) => {
  // Ако заявката не е за API endpoint и не е за статичен файл
  // (т.е. няма file extension), пренасочваме към index.html
  if (!req.path.startsWith("/api") && !req.path.includes(".")) {
    return res.sendFile(path.join(frontendBuildPath, "index.html"));
  }
  next();
};

module.exports = {
  staticMiddleware,
  fallbackMiddleware,
  frontendBuildPath,
};

