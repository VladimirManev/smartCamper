import React, { useState, useEffect } from "react";
import { ThemeProvider, createTheme } from "@mui/material/styles";
import {
  CssBaseline,
  AppBar,
  Toolbar,
  Typography,
  Container,
  Card,
  CardContent,
  Box,
  Chip,
  LinearProgress,
} from "@mui/material";

const theme = createTheme({
  palette: {
    mode: "dark",
    primary: {
      main: "#4caf50",
    },
    secondary: {
      main: "#2196f3",
    },
  },
});

interface SensorData {
  value: number;
  unit: string;
  timestamp: string;
}

interface AllSensorData {
  temperature: { [key: string]: SensorData };
  humidity: { [key: string]: SensorData };
  waterLevel: { [key: string]: SensorData };
  battery: { [key: string]: SensorData };
}

function App() {
  const [apiConnected, setApiConnected] = useState(false);
  const [sensorData, setSensorData] = useState<AllSensorData>({
    temperature: {},
    humidity: {},
    waterLevel: {},
    battery: {},
  });

  useEffect(() => {
    // Проверка на API връзката и зареждане на данни
    const loadData = async () => {
      try {
        const response = await fetch("http://localhost:3000/api/status");
        if (response.ok) {
          setApiConnected(true);

          // Зареждане на сензорни данни
          const sensorsResponse = await fetch(
            "http://localhost:3000/api/sensors"
          );
          if (sensorsResponse.ok) {
            const data = await sensorsResponse.json();
            if (data.success && data.data) {
              setSensorData(data.data);
            }
          }
        }
      } catch (error) {
        console.error("API connection failed:", error);
        setApiConnected(false);
      }
    };

    loadData();

    // Обновяване на данните на всеки 5 секунди
    const interval = setInterval(loadData, 5000);

    return () => clearInterval(interval);
  }, []);

  const getStatusColor = (connected: boolean) =>
    connected ? "success" : "error";

  // Безопасни функции за достъп до данни
  const getTemperatureData = () => sensorData?.temperature || {};
  const getHumidityData = () => sensorData?.humidity || {};
  const getWaterLevelData = () => sensorData?.waterLevel || {};
  const getBatteryData = () => sensorData?.battery || {};

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <AppBar position="static">
        <Toolbar>
          <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
            🏕️ SmartCamper - Дашборд
          </Typography>
          <Chip
            label={`API: ${apiConnected ? "Онлайн" : "Офлайн"}`}
            color={getStatusColor(apiConnected)}
            size="small"
          />
        </Toolbar>
      </AppBar>

      <Container maxWidth="lg" sx={{ mt: 4 }}>
        {/* Статус на системата */}
        <Card sx={{ mb: 3 }}>
          <CardContent>
            <Typography variant="h6" gutterBottom>
              Статус на системата
            </Typography>
            <Box sx={{ display: "flex", gap: 2, mb: 2 }}>
              <Chip
                label={`API: ${apiConnected ? "Онлайн" : "Офлайн"}`}
                color={getStatusColor(apiConnected)}
              />
              <Chip label="ESP32: Активен" color="success" />
            </Box>
            <LinearProgress
              variant="determinate"
              value={apiConnected ? 100 : 50}
              sx={{ height: 8, borderRadius: 4 }}
            />
          </CardContent>
        </Card>

        {/* Сензорни данни */}
        <Box
          sx={{
            display: "grid",
            gridTemplateColumns: "repeat(auto-fit, minmax(300px, 1fr))",
            gap: 3,
          }}
        >
          {/* Температура */}
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                🌡️ Температура
              </Typography>
              {Object.entries(getTemperatureData()).map(([deviceId, data]) => (
                <Box key={deviceId} sx={{ mb: 2 }}>
                  <Typography variant="body2" color="text.secondary">
                    {deviceId === "living" ? "Жилищна зона" : deviceId}
                  </Typography>
                  <Typography variant="h4" component="div">
                    {data.value.toFixed(1)}°C
                  </Typography>
                  <Typography variant="caption" color="text.secondary">
                    Последна актуализация:{" "}
                    {new Date(data.timestamp).toLocaleTimeString()}
                  </Typography>
                </Box>
              ))}
              {Object.keys(getTemperatureData()).length === 0 && (
                <Typography color="text.secondary">
                  Няма данни за температура
                </Typography>
              )}
            </CardContent>
          </Card>

          {/* Влажност */}
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                💧 Влажност
              </Typography>
              {Object.entries(getHumidityData()).map(([deviceId, data]) => (
                <Box key={deviceId} sx={{ mb: 2 }}>
                  <Typography variant="body2" color="text.secondary">
                    {deviceId === "living" ? "Жилищна зона" : deviceId}
                  </Typography>
                  <Typography variant="h4" component="div">
                    {data.value.toFixed(1)}%
                  </Typography>
                  <Typography variant="caption" color="text.secondary">
                    Последна актуализация:{" "}
                    {new Date(data.timestamp).toLocaleTimeString()}
                  </Typography>
                </Box>
              ))}
              {Object.keys(getHumidityData()).length === 0 && (
                <Typography color="text.secondary">
                  Няма данни за влажност
                </Typography>
              )}
            </CardContent>
          </Card>

          {/* Ниво на водата */}
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                🚰 Ниво на водата
              </Typography>
              {Object.entries(getWaterLevelData()).map(([deviceId, data]) => (
                <Box key={deviceId} sx={{ mb: 2 }}>
                  <Typography variant="body2" color="text.secondary">
                    {deviceId === "tank" ? "Резервоар" : deviceId}
                  </Typography>
                  <Typography variant="h4" component="div">
                    {data.value.toFixed(1)}%
                  </Typography>
                  <LinearProgress
                    variant="determinate"
                    value={data.value}
                    sx={{ height: 8, borderRadius: 4, mb: 1 }}
                  />
                  <Typography variant="caption" color="text.secondary">
                    Последна актуализация:{" "}
                    {new Date(data.timestamp).toLocaleTimeString()}
                  </Typography>
                </Box>
              ))}
              {Object.keys(getWaterLevelData()).length === 0 && (
                <Typography color="text.secondary">
                  Няма данни за ниво на водата
                </Typography>
              )}
            </CardContent>
          </Card>

          {/* Батерия */}
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                🔋 Батерия
              </Typography>
              {Object.entries(getBatteryData()).map(([deviceId, data]) => (
                <Box key={deviceId} sx={{ mb: 2 }}>
                  <Typography variant="body2" color="text.secondary">
                    {deviceId === "main" ? "Основна" : deviceId}
                  </Typography>
                  <Typography variant="h4" component="div">
                    {data.value.toFixed(1)}%
                  </Typography>
                  <LinearProgress
                    variant="determinate"
                    value={data.value}
                    sx={{ height: 8, borderRadius: 4, mb: 1 }}
                  />
                  <Typography variant="caption" color="text.secondary">
                    Последна актуализация:{" "}
                    {new Date(data.timestamp).toLocaleTimeString()}
                  </Typography>
                </Box>
              ))}
              {Object.keys(getBatteryData()).length === 0 && (
                <Typography color="text.secondary">
                  Няма данни за батерията
                </Typography>
              )}
            </CardContent>
          </Card>
        </Box>
      </Container>
    </ThemeProvider>
  );
}

export default App;
