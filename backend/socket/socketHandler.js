// Socket.io Handler
// Обработка на WebSocket комуникация

const setupSocketIO = (io) => {
  // Когато някой се свърже
  io.on("connection", (socket) => {
    console.log("✅ Frontend се свърза с WebSocket");

    // Изпращаме начални данни (примерни)
    socket.emit("sensorUpdate", {
      temperature: 25.5,
      humidity: 60,
      timestamp: new Date().toISOString(),
    });

    // Симулираме промяна на данни на всеки 3 секунди (за тест)
    const interval = setInterval(() => {
      const randomTemp = (20 + Math.random() * 10).toFixed(1);
      socket.emit("sensorUpdate", {
        temperature: parseFloat(randomTemp),
        humidity: 60,
        timestamp: new Date().toISOString(),
      });
    }, 3000);

    // Когато frontend се изключи
    socket.on("disconnect", () => {
      console.log("❌ Frontend се изключи");
      clearInterval(interval);
    });
  });
};

module.exports = setupSocketIO;
