import { useEffect, useRef } from "react";

/**
 * CardModal Component
 * Reusable modal component for card settings
 * @param {Object} props - Component props
 * @param {boolean} props.isOpen - Whether modal is open
 * @param {Function} props.onClose - Close handler
 * @param {string} props.title - Modal title
 * @param {ReactNode} props.children - Modal content
 */
export const CardModal = ({ isOpen, onClose, title, children }) => {
  const modalRef = useRef(null);
  const overlayRef = useRef(null);

  // Handle click outside to close
  useEffect(() => {
    if (!isOpen) return;

    const handleClickOutside = (event) => {
      if (
        modalRef.current &&
        !modalRef.current.contains(event.target) &&
        overlayRef.current &&
        overlayRef.current.contains(event.target)
      ) {
        event.preventDefault();
        event.stopPropagation();
        onClose();
      }
    };

    // Add event listener with small delay to avoid immediate trigger
    const timeoutId = setTimeout(() => {
      document.addEventListener("mousedown", handleClickOutside, true); // Use capture phase
      document.addEventListener("touchstart", handleClickOutside, true); // Use capture phase
    }, 100);

    return () => {
      clearTimeout(timeoutId);
      document.removeEventListener("mousedown", handleClickOutside, true);
      document.removeEventListener("touchstart", handleClickOutside, true);
    };
  }, [isOpen, onClose]);

  // Prevent body scroll when modal is open
  useEffect(() => {
    if (isOpen) {
      document.body.style.overflow = "hidden";
    } else {
      document.body.style.overflow = "";
    }
    return () => {
      document.body.style.overflow = "";
    };
  }, [isOpen]);

  if (!isOpen) return null;

  // Handle overlay click - stop propagation to prevent triggering elements below
  const handleOverlayClick = (e) => {
    if (e.target === overlayRef.current) {
      e.stopPropagation();
    }
  };

  return (
    <div className="modal-overlay" ref={overlayRef} onClick={handleOverlayClick}>
      <div className="modal-container" ref={modalRef} onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <h2 className="modal-title">{title}</h2>
          <button className="modal-close-button" onClick={onClose} aria-label="Close">
            <svg
              width="20"
              height="20"
              viewBox="0 0 24 24"
              fill="none"
              stroke="currentColor"
              strokeWidth="2"
              strokeLinecap="round"
              strokeLinejoin="round"
            >
              <line x1="18" y1="6" x2="6" y2="18"></line>
              <line x1="6" y1="6" x2="18" y2="18"></line>
            </svg>
          </button>
        </div>
        <div className="modal-content">{children}</div>
      </div>
    </div>
  );
};

