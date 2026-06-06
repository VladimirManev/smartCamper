/**
 * SceneCard — preset tile inside the Scenes modal (same look as main menu cards).
 */

import { Card } from "./Card";

function getSceneIcon(icon) {
  switch (icon) {
    case "house":
      return <i className="fas fa-house" aria-hidden />;
    case "car":
      return <i className="fas fa-car" aria-hidden />;
    case "film":
      return <i className="fas fa-film" aria-hidden />;
    case "bed":
      return <i className="fas fa-bed" aria-hidden />;
    case "utensils":
      return <i className="fas fa-utensils" aria-hidden />;
    case "power-off":
      return <i className="fas fa-power-off" aria-hidden />;
    default:
      return <i className="fas fa-circle" aria-hidden />;
  }
}

/**
 * @param {Object} props
 * @param {{ id: string, label: string, icon: string }} props.scene
 * @param {Function} [props.onClick]
 * @param {boolean} [props.disabled]
 * @param {boolean} [props.selected]
 */
export function SceneCard({ scene, onClick, disabled = false, selected = false }) {
  return (
    <Card
      name={scene.label}
      icon={getSceneIcon(scene.icon)}
      buttonState={selected ? "on" : "off"}
      iconState="active"
      onClick={onClick}
      disabled={disabled}
    />
  );
}
