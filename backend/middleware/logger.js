// Logger Middleware
// Логва всички HTTP заявки за debugging

const loggerMiddleware = (req, res, next) => {
  // Записваме времето на заявката
  const timestamp = new Date().toISOString();

  // Логваме детайлите на заявката
  console.log(`📨 ${timestamp} - ${req.method} ${req.url}`);

  // Ако има body данни (POST/PUT), логваме ги
  if (req.body && Object.keys(req.body).length > 0) {
    console.log(`📦 Body:`, req.body);
  }

  // Предаваме контрола на следващия middleware
  next();
};

module.exports = loggerMiddleware;
