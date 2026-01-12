import { useEffect, useRef, useState } from "react";

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
  const [isClosing, setIsClosing] = useState(false);

  // Handle click outside to close
  const handleOverlayClick = (event) => {
    // Only close if clicking directly on overlay, not on modal content
    if (event.target === overlayRef.current) {
      event.preventDefault();
      event.stopPropagation();
      handleClose();
    }
  };

  // Handle close with animation
  const handleClose = () => {
    setIsClosing(true);
    setTimeout(() => {
      setIsClosing(false);
      onClose();
    }, 500); // Match animation duration
  };

  // Block click events on elements below overlay using capture phase
  useEffect(() => {
    if (!isOpen) return;

    const blockClicks = (event) => {
      // Allow clicks on modal elements (modal container and its children)
      if (modalRef.current && modalRef.current.contains(event.target)) {
        return;
      }
      // Allow clicks on overlay itself (for closing)
      if (overlayRef.current && event.target === overlayRef.current) {
        return;
      }
      // Block all other clicks (elements below overlay)
      event.stopPropagation();
      event.preventDefault();
      event.stopImmediatePropagation();
    };

    // Use capture phase to catch events before they reach elements below
    // Only block mousedown/touchstart to prevent clicks from being registered
    document.addEventListener('mousedown', blockClicks, true);
    document.addEventListener('touchstart', blockClicks, true);

    return () => {
      document.removeEventListener('mousedown', blockClicks, true);
      document.removeEventListener('touchstart', blockClicks, true);
    };
  }, [isOpen]);

  // Prevent body scroll when modal is open
  useEffect(() => {
    if (isOpen) {
      document.body.style.overflow = "hidden";
      setIsClosing(false);
    } else {
      document.body.style.overflow = "";
    }
    return () => {
      document.body.style.overflow = "";
    };
  }, [isOpen]);

  if (!isOpen) return null;

  return (
    <div 
      className={`modal-overlay ${isClosing ? "closing" : ""}`}
      ref={overlayRef} 
      onClick={handleOverlayClick}
    >
      <div 
        className={`modal-container ${isClosing ? "closing" : ""}`}
        ref={modalRef} 
        onClick={(e) => e.stopPropagation()}
      >
        <div className="modal-header">
          <h2 className="modal-title">{title}</h2>
          <button className="modal-close-button" onClick={handleClose} aria-label="Close">
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

