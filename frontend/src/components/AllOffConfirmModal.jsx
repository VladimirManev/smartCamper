/**
 * One-tap confirmation after long-press on Scenes card.
 */

import { CardModal } from "./CardModal";

/**
 * @param {Object} props
 * @param {boolean} props.isOpen
 * @param {() => void} props.onClose
 * @param {() => void} props.onConfirm
 */
export function AllOffConfirmModal({ isOpen, onClose, onConfirm }) {
  const handleConfirm = () => {
    onConfirm();
    onClose();
  };

  return (
    <CardModal isOpen={isOpen} onClose={onClose} title="All Off" zIndex={1100} compact>
      <div className="all-off-confirm">
        <p className="all-off-confirm__message">
          Turn everything off?
        </p>
        <div className="all-off-confirm__actions">
          <button
            type="button"
            className="scene-drive-btn scene-drive-btn--primary all-off-confirm__confirm"
            onClick={handleConfirm}
          >
            Turn everything off
          </button>
          <button
            type="button"
            className="scene-drive-btn scene-drive-btn--secondary"
            onClick={onClose}
          >
            Cancel
          </button>
        </div>
      </div>
    </CardModal>
  );
}
