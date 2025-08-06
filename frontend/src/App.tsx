import React, { useEffect, useState } from "react";
import {
  ThemeProvider,
  createTheme,
  CssBaseline,
  Box,
  Container,
  AppBar,
  Toolbar,
  Typography,
  Card,
  CardContent,
  Chip,
  Alert,
} from "@mui/material";
import { Wifi as WifiIcon, WifiOff as WifiOffIcon } from "@mui/icons-material";

// Създаваме тема за кемпера
const theme = createTheme({
  palette: {
    mode: "light",
    primary: {
      main: "#2e7d32", // Тъмно зелено
    },
    secondary: {
      main: "#1976d2", // Синьо
    },
    background: {
      default: "#f5f5f5",
      paper: "#ffffff",
    },
  },
});

function App() {
  const [mqttConnected, setMqttConnected] = useState(false);
  const [apiConnected, setApiConnected] = useState(false);

  // Проверка на API връзката
  useEffect(() => {
    const checkAPI = async () => {
      try {
        const response = await fetch("http://localhost:3000/api/status");
        if (response.ok) {
          setApiConnected(true);
        }
      } catch (error) {
        console.error("API грешка:", error);
        setApiConnected(false);
      }
    };

    checkAPI();
    const interval = setInterval(checkAPI, 5000); // Проверяваме на всеки 5 секунди

    return () => clearInterval(interval);
  }, []);

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />

      {/* Header */}
      <AppBar position="static" elevation={0}>
        <Toolbar>
          <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
            🏕️ SmartCamper
          </Typography>

          {/* Статус индикатори */}
          <Box sx={{ display: "flex", gap: 1, alignItems: "center" }}>
            <Chip
              icon={mqttConnected ? <WifiIcon /> : <WifiOffIcon />}
              label={mqttConnected ? "MQTT" : "MQTT Offline"}
              color={mqttConnected ? "success" : "error"}
              size="small"
            />
            <Chip
              icon={apiConnected ? <WifiIcon /> : <WifiOffIcon />}
              label={apiConnected ? "API" : "API Offline"}
              color={apiConnected ? "success" : "error"}
              size="small"
            />
          </Box>
        </Toolbar>
      </AppBar>

      {/* Основно съдържание */}
      <Container maxWidth="lg" sx={{ mt: 3, mb: 3 }}>
        <Box sx={{ display: "flex", flexDirection: "column", gap: 3 }}>
          {/* Системен статус */}
          <Card>
            <CardContent>
              <Typography variant="h5" component="h2" gutterBottom>
                📊 Системен статус
              </Typography>

              <Box sx={{ display: "flex", flexWrap: "wrap", gap: 2 }}>
                <Box sx={{ flex: "1 1 200px", textAlign: "center", p: 2 }}>
                  <Chip
                    icon={mqttConnected ? <WifiIcon /> : <WifiOffIcon />}
                    label={mqttConnected ? "MQTT Online" : "MQTT Offline"}
                    color={mqttConnected ? "success" : "error"}
                    sx={{ mb: 1 }}
                  />
                  <Typography variant="body2" color="text.secondary">
                    Real-time комуникация
                  </Typography>
                </Box>

                <Box sx={{ flex: "1 1 200px", textAlign: "center", p: 2 }}>
                  <Chip
                    icon={apiConnected ? <WifiIcon /> : <WifiOffIcon />}
                    label={apiConnected ? "API Online" : "API Offline"}
                    color={apiConnected ? "success" : "error"}
                    sx={{ mb: 1 }}
                  />
                  <Typography variant="body2" color="text.secondary">
                    HTTP API връзка
                  </Typography>
                </Box>

                <Box sx={{ flex: "1 1 200px", textAlign: "center", p: 2 }}>
                  <Typography variant="h6" color="primary">
                    v1.0.0
                  </Typography>
                  <Typography variant="body2" color="text.secondary">
                    Версия на системата
                  </Typography>
                </Box>

                <Box sx={{ flex: "1 1 200px", textAlign: "center", p: 2 }}>
                  <Typography variant="h6" color="primary">
                    🚀
                  </Typography>
                  <Typography variant="body2" color="text.secondary">
                    Системата работи
                  </Typography>
                </Box>
              </Box>
            </CardContent>
          </Card>

          {/* Сензорни данни */}
          <Card>
            <CardContent>
              <Typography variant="h5" component="h2" gutterBottom>
                📡 Сензорни данни
              </Typography>

              {!mqttConnected && (
                <Alert severity="warning" sx={{ mb: 2 }}>
                  MQTT връзката не е налична. Данните не се обновяват в реално
                  време.
                </Alert>
              )}

              <Box sx={{ display: "flex", flexWrap: "wrap", gap: 3 }}>
                <Box sx={{ flex: "1 1 250px" }}>
                  <Card sx={{ textAlign: "center", p: 2 }}>
                    <Typography variant="h4" sx={{ mb: 1 }}>
                      🌡️
                    </Typography>
                    <Typography variant="h6" gutterBottom>
                      Температура
                    </Typography>
                    <Typography
                      variant="h3"
                      color="error"
                      sx={{ fontWeight: "bold" }}
                    >
                      --°C
                    </Typography>
                    <Typography variant="body2" color="text.secondary">
                      Няма данни
                    </Typography>
                  </Card>
                </Box>

                <Box sx={{ flex: "1 1 250px" }}>
                  <Card sx={{ textAlign: "center", p: 2 }}>
                    <Typography variant="h4" sx={{ mb: 1 }}>
                      💧
                    </Typography>
                    <Typography variant="h6" gutterBottom>
                      Влажност
                    </Typography>
                    <Typography
                      variant="h3"
                      color="info"
                      sx={{ fontWeight: "bold" }}
                    >
                      --%
                    </Typography>
                    <Typography variant="body2" color="text.secondary">
                      Няма данни
                    </Typography>
                  </Card>
                </Box>

                <Box sx={{ flex: "1 1 250px" }}>
                  <Card sx={{ textAlign: "center", p: 2 }}>
                    <Typography variant="h4" sx={{ mb: 1 }}>
                      🚰
                    </Typography>
                    <Typography variant="h6" gutterBottom>
                      Резервоар
                    </Typography>
                    <Typography
                      variant="h3"
                      color="primary"
                      sx={{ fontWeight: "bold" }}
                    >
                      --%
                    </Typography>
                    <Typography variant="body2" color="text.secondary">
                      Няма данни
                    </Typography>
                  </Card>
                </Box>

                <Box sx={{ flex: "1 1 250px" }}>
                  <Card sx={{ textAlign: "center", p: 2 }}>
                    <Typography variant="h4" sx={{ mb: 1 }}>
                      🔋
                    </Typography>
                    <Typography variant="h6" gutterBottom>
                      Батерия
                    </Typography>
                    <Typography
                      variant="h3"
                      color="warning"
                      sx={{ fontWeight: "bold" }}
                    >
                      --V
                    </Typography>
                    <Typography variant="body2" color="text.secondary">
                      Няма данни
                    </Typography>
                  </Card>
                </Box>
              </Box>

              <Box sx={{ textAlign: "center", p: 4, mt: 2 }}>
                <Typography variant="body1" color="text.secondary">
                  Няма налични сензорни данни
                </Typography>
                <Typography variant="body2" color="text.secondary">
                  Уверете се, че ESP32 модулите са свързани и изпращат данни
                </Typography>
              </Box>
            </CardContent>
          </Card>
        </Box>
      </Container>
    </ThemeProvider>
  );
}

export default App;
