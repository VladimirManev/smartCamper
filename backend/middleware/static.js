// Static Files Middleware
// Serves static files from React build and redirects all routes to index.html

const path = require("path");
const express = require("express");

// Path to React application build directory
const frontendBuildPath = path.join(__dirname, "../../frontend/dist");

// Middleware for serving static files (JS, CSS, images, etc.)
const staticMiddleware = express.static(frontendBuildPath, {
  // Cache headers for static files (1 year)
  maxAge: "1y",
  // ETag for optimization
  etag: true,
  // Last-Modified header
  lastModified: true,
});

// Fallback middleware - redirects all routes to index.html
// This is necessary for React Router (client-side routing)
const fallbackMiddleware = (req, res, next) => {
  // If request is not for API endpoint and not for static file
  // (i.e. no file extension), redirect to index.html
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
