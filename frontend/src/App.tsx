import React, { useState, useEffect, useMemo, useCallback } from "react";
import mqtt from "mqtt";
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
  DirectionsCar as CarIcon,
  DirectionsCarFilled as CarBackIcon,
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
  sensor_type?: string;
  device_id?: string;
}

interface AllSensorData {
  temperature: { [key: string]: SensorData };
  humidity: { [key: string]: SensorData };
  waterLevel: { [key: string]: SensorData };
  battery: { [key: string]: SensorData };
  tilt: { [key: string]: { roll?: SensorData; pitch?: SensorData } };
}

function App() {
  const [mqttConnected, setMqttConnected] = useState(false);
  const [currentTime, setCurrentTime] = useState(new Date());
  const [sensorData, setSensorData] = useState<AllSensorData>({
    temperature: {},
    humidity: {},
    waterLevel: {},
    battery: {},
    tilt: {},
  });

  useEffect(() => {
    // Обновяване на часа всяка секунда
    const timeInterval = setInterval(() => {
      setCurrentTime(new Date());
    }, 1000);

    // MQTT over WebSocket връзка
    const client = mqtt.connect("ws://localhost:3000/mqtt", {
      clientId: "frontend_" + Math.random().toString(16).substr(2, 8),
      clean: true,
      reconnectPeriod: 1000,
      connectTimeout: 30 * 1000,
      protocolVersion: 4,
    });

    client.on("connect", () => {
      console.log("🔌 MQTT свързан");
      setMqttConnected(true);

      // Абониране за всички сензорни данни
      client.subscribe(
        [
          "smartcamper/sensors/tilt/+/data",
          "smartcamper/sensors/temperature/+/data",
          "smartcamper/sensors/humidity/+/data",
        ],
        (err) => {
          if (err) {
            console.error("❌ Грешка при абониране:", err);
          } else {
            console.log("📡 Абониран за сензорни данни");
          }
        }
      );
    });

    client.on("message", (topic, message) => {
      try {
        const data = JSON.parse(message.toString());
        const parts = topic.split("/");

        if (parts[0] === "smartcamper" && parts[1] === "sensors") {
          const sensorType = parts[2];
          const deviceId = parts[3];

          setSensorData((prevData) => {
            const newData = { ...prevData };

            if (sensorType === "tilt") {
              console.log("📐 MQTT tilt съобщение:", topic, data);
              const sensorSubType =
                (data.sensor_type as "roll" | "pitch") || "roll";
              const prevTiltForDevice = newData.tilt[deviceId] || {};
              const updatedTiltForDevice = {
                ...prevTiltForDevice,
                [sensorSubType]: {
                  value: Number(data.value),
                  unit: data.unit,
                  sensor_type: sensorSubType,
                  device_id: data.device_id,
                  timestamp: new Date().toISOString(),
                },
              } as { roll?: SensorData; pitch?: SensorData };
              newData.tilt = {
                ...newData.tilt,
                [deviceId]: updatedTiltForDevice,
              };
            } else {
              const typedSensorType = sensorType as keyof Omit<
                AllSensorData,
                "tilt"
              >;
              newData[typedSensorType] = {
                ...newData[typedSensorType],
                [deviceId]: {
                  value: data.value,
                  unit: data.unit,
                  device_id: data.device_id,
                  timestamp: new Date().toISOString(),
                },
              };
            }

            return newData;
          });
        }
      } catch (error) {
        console.error("❌ Грешка при обработка на MQTT съобщение:", error);
      }
    });

    client.on("error", (error) => {
      console.error("❌ MQTT грешка:", error);
      setMqttConnected(false);
    });

    client.on("close", () => {
      console.log("🔌 MQTT връзката се затвори");
      setMqttConnected(false);
    });

    return () => {
      clearInterval(timeInterval);
      client.end();
    };
  }, []);

  const getStatusColor = (connected: boolean) =>
    connected ? "success" : "error";

  const getTemperatureData = () => sensorData?.temperature || {};
  const getHumidityData = () => sensorData?.humidity || {};
  const getWaterLevelData = () => sensorData?.waterLevel || {};
  const getBatteryData = () => sensorData?.battery || {};
  const getTiltData = useCallback(
    () => sensorData?.tilt || {},
    [sensorData?.tilt]
  );

  // Оптимизирани изчисления за наклоните (без да зависим от конкретен deviceId)
  const tiltForFirstDevice = useMemo(() => {
    const tilt = getTiltData();
    const firstKey = Object.keys(tilt)[0];
    return (firstKey ? tilt[firstKey] : {}) as {
      roll?: SensorData;
      pitch?: SensorData;
    };
  }, [getTiltData]);
  const rollValue = useMemo(
    () => tiltForFirstDevice?.roll?.value || 0,
    [tiltForFirstDevice]
  );
  const pitchValue = useMemo(
    () => tiltForFirstDevice?.pitch?.value || 0,
    [tiltForFirstDevice]
  );

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
        <Box
          sx={{
            display: "flex",
            flexDirection: "column",
            alignItems: "center",
            gap: 1,
          }}
        >
          {/* Кола в профил (Roll) */}
          <Box
            sx={{
              transform: `rotate(${rollValue}deg)`,
              transition: "transform 0.1s ease-out",
              display: "flex",
              alignItems: "center",
              justifyContent: "center",
            }}
          >
            <CarIcon sx={{ fontSize: 32, color: "#ff9800" }} />
          </Box>
          {/* Кола отзад (Pitch) */}
          <Box
            sx={{
              transform: `rotate(${pitchValue}deg)`,
              transition: "transform 0.1s ease-out",
              display: "flex",
              alignItems: "center",
              justifyContent: "center",
            }}
          >
            <CarBackIcon sx={{ fontSize: 32, color: "#ff9800" }} />
          </Box>
        </Box>
      ),
      title: "Наклон",
      value: null,
      unit: "",
      color: "#ff9800",
      customContent: (
        <Box sx={{ textAlign: "center" }}>
          <Typography
            variant="body2"
            sx={{ color: "#ff9800", fontWeight: 600 }}
          >
            Roll: {Math.abs(rollValue)}°
          </Typography>
          <Typography
            variant="body2"
            sx={{ color: "#ff9800", fontWeight: 600 }}
          >
            Pitch: {Math.abs(pitchValue)}°
          </Typography>
        </Box>
      ),
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
            {mqttConnected ? (
              <WifiIcon sx={{ fontSize: 24, color: "#4caf50", mr: 1 }} />
            ) : (
              <WifiOffIcon sx={{ fontSize: 24, color: "#f44336", mr: 1 }} />
            )}
            <Typography
              variant="caption"
              sx={{ color: mqttConnected ? "#4caf50" : "#f44336" }}
            >
              MQTT {mqttConnected ? "Онлайн" : "Офлайн"}
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
                  {item.title === "Roll" || item.title === "Pitch"
                    ? Math.round(item.value) + item.unit
                    : item.value.toFixed(1) + item.unit}
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
