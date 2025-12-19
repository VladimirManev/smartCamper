// CORS Middleware
// Allows frontend to communicate with backend

const corsMiddleware = (req, res, next) => {
  // Allow requests from any domain
  res.header("Access-Control-Allow-Origin", "*");

  // Allow specific headers
  res.header("Access-Control-Allow-Headers", "Content-Type, Authorization");

  // Allow specific HTTP methods
  res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");

  // If OPTIONS request (preflight), return immediately
  if (req.method === "OPTIONS") {
    res.sendStatus(200);
    return;
  }

  // Pass control to next middleware
  next();
};

module.exports = corsMiddleware;
