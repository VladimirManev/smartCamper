/**
 * Scenes modal — scene grid or full-modal action picker per scene.
 */

import { useState } from "react";
import { SCENES } from "../constants/scenes";
import { AllOffScenePanel } from "./AllOffScenePanel";
import { CookingScenePanel } from "./CookingScenePanel";
import { DriveScenePanel } from "./DriveScenePanel";
import { FilmScenePanel } from "./FilmScenePanel";
import { NormalScenePanel } from "./NormalScenePanel";
import { SleepScenePanel } from "./SleepScenePanel";
import { SceneCard } from "./SceneCard";

const SCENES_WITH_ACTION_PANEL = new Set(["normal", "drive", "film", "sleep", "cooking", "all-off"]);

/**
 * @param {Object} props
 * @param {(sceneId: string) => void} props.onSceneSelect
 * @param {(options: import("./NormalScenePanel").NormalScenePayload) => void} props.onApplyNormal
 * @param {(options: import("./DriveScenePanel").DriveScenePayload) => void} props.onApplyDrive
 * @param {(options: import("./FilmScenePanel").FilmScenePayload) => void} props.onApplyFilm
 * @param {(options: import("../utils/applyScene").SleepScenePayload) => void} props.onApplySleep
 * @param {(options: import("./CookingScenePanel").CookingScenePayload) => void} props.onApplyCooking
 * @param {(options: import("./AllOffScenePanel").AllOffScenePayload) => void} props.onApplyAllOff
 * @param {(sceneId: string) => boolean} props.isSceneDisabled
 */
export function ScenesModalContent({
  onSceneSelect,
  onApplyNormal,
  onApplyDrive,
  onApplyFilm,
  onApplySleep,
  onApplyCooking,
  onApplyAllOff,
  isSceneDisabled,
}) {
  const [selectedSceneId, setSelectedSceneId] = useState(null);

  const handleSceneClick = (sceneId) => {
    if (SCENES_WITH_ACTION_PANEL.has(sceneId)) {
      setSelectedSceneId(sceneId);
      return;
    }
    onSceneSelect(sceneId);
  };

  if (selectedSceneId === "normal") {
    return (
      <NormalScenePanel
        onApply={onApplyNormal}
        onBack={() => setSelectedSceneId(null)}
      />
    );
  }

  if (selectedSceneId === "drive") {
    return (
      <DriveScenePanel
        onApply={onApplyDrive}
        onBack={() => setSelectedSceneId(null)}
      />
    );
  }

  if (selectedSceneId === "film") {
    return (
      <FilmScenePanel
        onApply={onApplyFilm}
        onBack={() => setSelectedSceneId(null)}
      />
    );
  }

  if (selectedSceneId === "sleep") {
    return (
      <SleepScenePanel
        onApply={onApplySleep}
        onBack={() => setSelectedSceneId(null)}
      />
    );
  }

  if (selectedSceneId === "cooking") {
    return (
      <CookingScenePanel
        onApply={onApplyCooking}
        onBack={() => setSelectedSceneId(null)}
      />
    );
  }

  if (selectedSceneId === "all-off") {
    return (
      <AllOffScenePanel
        onApply={onApplyAllOff}
        onBack={() => setSelectedSceneId(null)}
      />
    );
  }

  return (
    <div className="scenes-modal">
      <div className="modal-grid">
        {SCENES.map((scene) => (
          <div className="card-wrapper" key={scene.id}>
            <SceneCard
              scene={scene}
              onClick={() => handleSceneClick(scene.id)}
              disabled={isSceneDisabled(scene.id)}
            />
            <p className="card-label">{scene.label}</p>
          </div>
        ))}
      </div>
    </div>
  );
}
