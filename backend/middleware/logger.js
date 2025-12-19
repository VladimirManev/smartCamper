// Logger Middleware
// Logs all HTTP requests for debugging

const loggerMiddleware = (req, res, next) => {
  // Record request time
  const timestamp = new Date().toISOString();

  // Log request details
  console.log(`📨 ${timestamp} - ${req.method} ${req.url}`);

  // If body data exists (POST/PUT), log it
  if (req.body && Object.keys(req.body).length > 0) {
    console.log(`📦 Body:`, req.body);
  }

  // Pass control to next middleware
  next();
};

module.exports = loggerMiddleware;
