import React, { useState, useEffect } from "react";
import { ThemeProvider, createTheme } from "@mui/material/styles";
import { CssBaseline, Box, Typography, Paper, Chip } from "@mui/material";
import {
  Thermostat as ThermostatIcon,
  Opacity as OpacityIcon,
  WaterDrop as WaterDropIcon,
  Battery90 as BatteryIcon,
  Settings as SettingsIcon,
  AccessTime as ClockIcon,
  CalendarToday as CalendarIcon,
  Wifi as WifiIcon,
  WifiOff as WifiOffIcon,
} from "@mui/icons-material";

const theme = createTheme({
  palette: {
    mode: "light",
    primary: {
      main: "#1976d2",
    },
    secondary: {
      main: "#dc004e",
    },
    background: {
      default: "#ffffff",
      paper: "#ffffff",
    },
  },
  typography: {
    fontFamily: '"Roboto", "Helvetica", "Arial", sans-serif',
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
  const [currentTime, setCurrentTime] = useState(new Date());
  const [sensorData, setSensorData] = useState<AllSensorData>({
    temperature: {},
    humidity: {},
    waterLevel: {},
    battery: {},
  });

  useEffect(() => {
    // Обновяване на часа всяка секунда
    const timeInterval = setInterval(() => {
      setCurrentTime(new Date());
    }, 1000);

    // Проверка на API връзката и зареждане на данни
    const loadData = async () => {
      try {
        const response = await fetch("http://localhost:3000/api/status");
        if (response.ok) {
          setApiConnected(true);

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
    const dataInterval = setInterval(loadData, 5000);

    return () => {
      clearInterval(timeInterval);
      clearInterval(dataInterval);
    };
  }, []);

  const getStatusColor = (connected: boolean) =>
    connected ? "success" : "error";

  const getTemperatureData = () => sensorData?.temperature || {};
  const getHumidityData = () => sensorData?.humidity || {};
  const getWaterLevelData = () => sensorData?.waterLevel || {};
  const getBatteryData = () => sensorData?.battery || {};

  const formatTime = (date: Date) => {
    return date.toLocaleTimeString("bg-BG", {
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
    });
  };

  const formatDate = (date: Date) => {
    return date.toLocaleDateString("bg-BG", {
      weekday: "long",
      year: "numeric",
      month: "long",
      day: "numeric",
    });
  };

  const menuItems = [
    {
      icon: <ThermostatIcon sx={{ fontSize: 40, color: "#ff6b35" }} />,
      title: "Температура",
      value: Object.values(getTemperatureData())[0]?.value || 0,
      unit: "°C",
      color: "#ff6b35",
    },
    {
      icon: <OpacityIcon sx={{ fontSize: 40, color: "#4ecdc4" }} />,
      title: "Влажност",
      value: Object.values(getHumidityData())[0]?.value || 0,
      unit: "%",
      color: "#4ecdc4",
    },
    {
      icon: <WaterDropIcon sx={{ fontSize: 40, color: "#45b7d1" }} />,
      title: "Вода",
      value: Object.values(getWaterLevelData())[0]?.value || 0,
      unit: "%",
      color: "#45b7d1",
    },
    {
      icon: <BatteryIcon sx={{ fontSize: 40, color: "#96ceb4" }} />,
      title: "Батерия",
      value: Object.values(getBatteryData())[0]?.value || 0,
      unit: "%",
      color: "#96ceb4",
    },
    {
      icon: (
        <Box sx={{ textAlign: "center" }}>
          <Box
            sx={{
              display: "flex",
              alignItems: "center",
              justifyContent: "center",
              mb: 1,
            }}
          >
            {apiConnected ? (
              <WifiIcon sx={{ fontSize: 24, color: "#4caf50", mr: 1 }} />
            ) : (
              <WifiOffIcon sx={{ fontSize: 24, color: "#f44336", mr: 1 }} />
            )}
            <Typography
              variant="caption"
              sx={{ color: apiConnected ? "#4caf50" : "#f44336" }}
            >
              API {apiConnected ? "Онлайн" : "Офлайн"}
            </Typography>
          </Box>
          <Box
            sx={{
              display: "flex",
              alignItems: "center",
              justifyContent: "center",
            }}
          >
            <WifiIcon sx={{ fontSize: 24, color: "#4caf50", mr: 1 }} />
            <Typography variant="caption" sx={{ color: "#4caf50" }}>
              MQTT Онлайн
            </Typography>
          </Box>
        </Box>
      ),
      title: "Статус",
      value: null,
      unit: "",
      color: "#2c3e50",
    },
    {
      icon: <ClockIcon sx={{ fontSize: 40, color: "#f7dc6f" }} />,
      title: "Час",
      value: null,
      unit: "",
      color: "#f7dc6f",
      customContent: (
        <Typography
          variant="h6"
          sx={{ color: "#f7dc6f", fontWeight: 700, fontFamily: "monospace" }}
        >
          {formatTime(currentTime)}
        </Typography>
      ),
    },
    {
      icon: <CalendarIcon sx={{ fontSize: 40, color: "#dda0dd" }} />,
      title: "Дата",
      value: null,
      unit: "",
      color: "#dda0dd",
      customContent: (
        <Box sx={{ textAlign: "center" }}>
          <Typography variant="h6" sx={{ color: "#dda0dd", fontWeight: 700 }}>
            {currentTime.getDate()}
          </Typography>
          <Typography variant="caption" sx={{ color: "#dda0dd" }}>
            {currentTime.toLocaleDateString("bg-BG", { month: "short" })}
          </Typography>
        </Box>
      ),
    },
    {
      icon: <SettingsIcon sx={{ fontSize: 40, color: "#a8a8a8" }} />,
      title: "Настройки",
      value: null,
      unit: "",
      color: "#a8a8a8",
    },
  ];

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />

      {/* Основен контейнер */}
      <Box
        sx={{
          minHeight: "100vh",
          background: "#ffffff",
          padding: 2,
        }}
      >
        {/* iPhone-стил меню с икони */}
        <Box
          sx={{
            display: "grid",
            gridTemplateColumns: {
              xs: "repeat(3, 1fr)",
              sm: "repeat(4, 1fr)",
              md: "repeat(4, 1fr)",
            },
            gap: 3,
            maxWidth: 1200,
            margin: "0 auto",
          }}
        >
          {menuItems.map((item, index) => (
            <Paper
              key={index}
              sx={{
                aspectRatio: "1 / 1",
                display: "flex",
                flexDirection: "column",
                alignItems: "center",
                justifyContent: "center",
                background: "#ffffff",
                borderRadius: 4,
                boxShadow: "0 4px 20px rgba(0, 0, 0, 0.1)",
                transition: "all 0.3s ease",
                cursor: "pointer",
                "&:hover": {
                  transform: "translateY(-5px)",
                  boxShadow: "0 8px 30px rgba(0, 0, 0, 0.15)",
                },
              }}
            >
              <Box sx={{ mb: 1 }}>{item.icon}</Box>
              <Typography
                variant="body2"
                sx={{
                  fontWeight: 500,
                  color: "#2c3e50",
                  textAlign: "center",
                  mb: 0.5,
                }}
              >
                {item.title}
              </Typography>
              {item.customContent ? (
                item.customContent
              ) : item.value !== null ? (
                <Typography
                  variant="h6"
                  sx={{
                    fontWeight: 700,
                    color: item.color,
                  }}
                >
                  {item.value.toFixed(1)}
                  {item.unit}
                </Typography>
              ) : null}
            </Paper>
          ))}
        </Box>
      </Box>
    </ThemeProvider>
  );
}

export default App;
