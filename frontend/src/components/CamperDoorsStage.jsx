/**
 * Bird's-eye camper with open-door stroke markers.
 * Shared by alarm idle and status overview.
 */

const CAMPER_IMAGE = "/camper_birth_view.png";

/**
 * Door marker positions as % of the van layer (image rotated -90°: nose up).
 * Optional dx/dy are extra pixel offsets in the van-layer coords.
 * `sensor` maps to doors.* from security status (rear drives both rear markers).
 */
const DOOR_MARKERS = [
  { id: "driver", sensor: "driver", left: 30, top: 18 },
  { id: "passenger", sensor: "passenger", left: 70, top: 18 },
  { id: "sliding", sensor: "sliding", left: 82, top: 52, dx: -45, dy: 60 },
  { id: "rear_left", sensor: "rear", left: 32, top: 97, dx: 5, dy: 10 },
  { id: "rear_right", sensor: "rear", left: 68, top: 97, dx: -10, dy: 10 },
];

export const DEFAULT_DOORS = {
  driver: false,
  passenger: false,
  sliding: false,
  rear: false,
};

/**
 * @param {Object} props
 * @param {Object} [props.doors] - { driver, passenger, sliding, rear }
 * @param {string} [props.stageClassName]
 * @param {string} [props.vanLayerClassName]
 */
export function CamperDoorsStage({
  doors = DEFAULT_DOORS,
  stageClassName = "",
  vanLayerClassName = "",
}) {
  const openMarkers = DOOR_MARKERS.filter((door) => !!doors[door.sensor]);

  return (
    <div
      className={`perimeter-modal__stage camper-doors__stage${
        stageClassName ? ` ${stageClassName}` : ""
      }`}
    >
      <div
        className={`perimeter-modal__van-layer camper-doors__van-layer${
          vanLayerClassName ? ` ${vanLayerClassName}` : ""
        }`}
      >
        {openMarkers.map((door) => (
          <span
            key={door.id}
            className={`camper-doors__marker camper-doors__marker--stroke camper-doors__marker--${door.id} camper-doors__marker--open`}
            style={{
              left: `calc(${door.left}% + ${door.dx || 0}px)`,
              top: `calc(${door.top}% + ${door.dy || 0}px)`,
            }}
            aria-hidden
          />
        ))}
        <img
          className="perimeter-modal__van-img"
          src={CAMPER_IMAGE}
          alt="Camper"
          draggable={false}
        />
      </div>
    </div>
  );
}
