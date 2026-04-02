# SmartCamper Backend (кратко на български)

Сървър на **Node.js**: **Express** (HTTP и статични файлове), **Socket.io** (WebSocket към браузъра), **Aedes** (MQTT брокер). Пренася съобщения от ESP32 модулите към отворения frontend и изпраща команди от UI към MQTT.

## Структура

- `server.js` — стартиране на HTTP, Socket.io и MQTT
- `socket/socketHandler.js` — връзки, мост MQTT → WebSocket, маршрутизиране на команди от клиента
- `socket/handlers/` — обработка по теми и **`moduleCommandHandler.js`** (в т.ч. `force_update`)
- `src/ModuleRegistry.js` — онлайн/офлайн състояние на модулите (heartbeat)

## WebSocket: от frontend към сървъра

| Събитие | Назначение |
|---------|------------|
| `ledCommand` | LED / релета (module-2) |
| `floorHeatingCommand` | Подово отопление (module-3) |
| `levelingCommand` | Нивелиране (module-3) |
| `damperCommand` | Клапи (module-4) |
| `tableCommand` | Маса (module-4) |
| `applianceCommand` | Уреди (module-5) |
| **`forceModuleUpdate`** | Форсирай **един** модул да публикува пресни данни |

### `forceModuleUpdate`

Кара бекенда да публикува по MQTT команда **`force_update`** към **избран** модул.

- **Payload:** `{ "moduleId": "module-1" }` — низът трябва да отговаря на шаблон `module-` + номер (напр. `module-1`, `module-5`).
- **Невалиден липсващ `moduleId`:** заявката се игнорира (без грешка към клиента).
- **MQTT topic:** `smartcamper/commands/<moduleId>/force_update`, тяло `{}`.
- **Код:** `socketHandler.js` → `sendForceUpdate` в `moduleCommandHandler.js`.

Ползва се от UI (напр. модал за сива вода), за да се обновява **само module-1** на интервал, без да се форсират всички модули.

При **ново свързване** на клиент сървърът по старому извиква **форс към всички онлайн модули** веднъж (с кратко закъснение).

## WebSocket: от сървъра към клиента

Основни събития: `moduleStatusUpdate`, `sensorUpdate`, `ledStatusUpdate`, `floorHeatingStatusUpdate`, `levelingData`, `damperStatusUpdate`, `tableStatusUpdate`, `applianceStatusUpdate` (подробности в английския `README.md`).

## Стартиране

```bash
cd backend
npm install
npm start
```

Пълна техническа документация (на английски): **`README.md`** в същата папка.
