# SmartCamper Frontend

React приложение за управление и мониторинг на SmartCamper системата.

## Технологии

- **React** - UI библиотека
- **Vite** - Build tool и dev server
- **Socket.io Client** - Real-time комуникация с backend
- **Font Awesome** - Икони

## Компоненти

### Основни компоненти

- **App.jsx** - Главен компонент, координира всички функционалности
- **StatusIcons** - Показва статус на backend връзката и WiFi сигнал индикатори за модулите
- **SignalIndicator** - Индикатор за WiFi сигнал (номер на модула + 4 чертички за сила на сигнала)
- **SensorCard** - Карта за показване на сензорни данни (температура, влажност)
- **GrayWaterTank** - Визуализация на нивото на сивата вода
- **LEDCard** - Контрол на LED ленти
- **FloorHeatingCard** - Контрол на подово отопление

### Custom Hooks

- **useSocket** - Управление на Socket.io връзката
- **useModuleStatus** - Следене на статуса на модулите (online/offline, RSSI)
- **useSensorData** - Получаване и управление на сензорни данни
- **useLEDController** - Управление на LED ленти и релета
- **useFloorHeating** - Управление на подово отопление
- **useBatterySystem** - Victron snapshot → energy diagram (вкл. AC charger live/stale)
- **useApplianceController** - Релета (уреди); fridge power (index 2) идва от module-6 чрез backend
- **useFridge** - Fridge BLE status (`fridgeStatusUpdate`) + `fridgeCommand`

### Fridge UI

- Картата е **inactive** когато **module-6** е offline; long-press power → module-6 реле
- `FridgeModalContent` — tap зона → +/− с **1.5 s** debounce; spinner докато ESP приложи командата (~20 s timeout)
- ECO/MAX със същия pending/confirm поток
- Age ред: колко секунди от последен MQTT + интервал; след **45 s** → Stale и тирета (—)
- Status carousel: read-only fridge slide; power OFF → „OFF“
- Подробности: `esp32-modules/module-6/FRIDGE_BLE.md`

### Boiler / 230 V

Бойлерът (relay 4) се управлява с правила в `src/constants/appliances.js`:

- Включване само при **инвертор ON** или **живо AC зарядно** (shore power от module-6)
- Гаси се автоматично само когато **и двата** 230 V източника липсват
- Подробности: `esp32-modules/module-5/README.md` → Boiler ↔ 230 V

## WiFi Сигнал Индикатор

Всеки модул показва индикатор за сила на WiFi сигнала:

- **Номер на модула** - Показва кой модул е (1, 2, 3)
- **4 чертички** - Първата е най-къса, последната най-висока
- **Цветове**:
  - **Син** - Модулът е online
  - **Червен** - Модулът е offline
- **Запълване на чертичките** според RSSI:
  - **4 чертички** (всички пълни): -30 до -50 dBm (отличен сигнал)
  - **3 чертички**: -50 до -60 dBm (много добър сигнал)
  - **2 чертички**: -60 до -70 dBm (добър сигнал)
  - **1 чертичка** (само първата): -70 до -80 dBm (слаб сигнал)
  - **0 чертички**: под -80 dBm или няма WiFi
- **Незапълнените чертички** са сиви

## Стартиране

```bash
cd frontend
npm install
npm run dev
```

Приложението стартира на `http://localhost:5174`

## Build за production

```bash
npm run build
```

Build файловете се генерират в `dist/` директорията.
