# SmartCamper - Управление на електрическата система

Интелигентна система за управление на електрическата система на кемпера с три основни компонента:

## Архитектура

### 1. **Backend (Мозък)**

- **Raspberry Pi 4** с Express.js сървър
- **MQTT Broker (Aedes)** за комуникация с модулите
- **Socket.io** за real-time WebSocket
- **MQTT ↔ WebSocket Bridge** между ESP32 и frontend
- **SQLite история** за графики (`GET /api/history/readings`)

### 2. **Frontend (Дашборд)**

- **React** (Vite) + Socket.io
- Responsive UI (телефон + tablet landscape)
- Енергийна диаграма, уреди, осветление, климат, охрана, fridge BLE контрол

### 3. **ESP32 Модули**

- **PlatformIO** + Arduino C++
- **MQTT** с heartbeat и `force_update`
- **Модули**:
  - **module-1** — вътрешна/външна температура, влажност, сива вода
  - **module-2** — LED ленти, motion, ambient реле
  - **module-3** — подово отопление + нивелиране
  - **module-4** — клапи + маса
  - **module-5** — релета за уреди (захранване хладилник, помпа, инвертор, …) + урина + tablet backlight
  - **module-6** — Victron Instant Readout (SmartShunt, MPPT, Orion, AC зарядно) + AAOBOSI fridge BLE
  - **module-7** — чиста вода
  - **module-8** — аларма (зони, сирена, CAN врати)

## Структура

```
smartCamper/
├── backend/
├── frontend/
├── esp32-modules/        # module-1 … module-8 (+ test)
├── CAN_SIGNALS.md
├── FUTURE_IDEAS.md
└── update-from-git.sh
```

## Стартиране

### Backend

```bash
cd backend && npm install && npm start
```

Порт **3000** (HTTP + WebSocket). MQTT обикновено **1883**.

### Frontend

```bash
cd frontend && npm install && npm run dev
```

Dev: **5174**. На Pi production build се сервира от backend.

### ESP32

```bash
cd esp32-modules/module-6   # пример
pio run --target upload
```

Подробности: `README.md` / `README_BG.md` във всеки модул.

## Комуникация

- **MQTT**: ESP32 ↔ Backend
- **WebSocket**: Frontend ↔ Backend
- Мост: MQTT → Socket.io (`sensorUpdate`, `victronStatusUpdate`, `fridgeStatusUpdate`, …)

Теми: `smartcamper/sensors/<moduleId>/…` и `smartcamper/commands/<moduleId>/…`.

## Документация

| Документ | Съдържание |
| -------- | ---------- |
| `backend/README.md` / `README_BG.md` | WebSocket, MQTT мост, history API |
| `esp32-modules/module-*/README*.md` | Хардуер и MQTT по модул |
| `esp32-modules/module-6/FRIDGE_BLE.md` | Fridge BLE протокол |
| `RASPBERRY_PI_COMMANDS.md` | Управление на Pi |
| `FUTURE_IDEAS.md` | Идеи за по-късно |
| `QUICK_START_PROMPT.md` | Правила за работа с AI |

Английски overview: `README.md`.

## Особености

- Real-time сензори, осветление, климат, уреди
- Victron енергия (солар, shunt, Orion, **AC зарядно**)
- Хладилник: захранване (module-5) + BLE режим/температури (module-6)
- Аларма (module-8)
- Офлайн: backend сервира frontend
- Автоматично WiFi / MQTT reconnect
