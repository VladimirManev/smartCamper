// CORS Middleware
// Позволява на фронтенда да комуникира с бекенда

const corsMiddleware = (req, res, next) => {
  // Позволяваме заявки от всеки домейн
  res.header("Access-Control-Allow-Origin", "*");

  // Позволяваме определени headers
  res.header("Access-Control-Allow-Headers", "Content-Type, Authorization");

  // Позволяваме определени HTTP методи
  res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");

  // Ако е OPTIONS заявка (preflight), връщаме веднага
  if (req.method === "OPTIONS") {
    res.sendStatus(200);
    return;
  }

  // Предаваме контрола на следващия middleware
  next();
};

module.exports = corsMiddleware;
