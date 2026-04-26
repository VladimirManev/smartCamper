/**
 * In-tablet-landscape detail panel: same chrome as CardModal but no overlay / body lock.
 */
export function EmbeddedModalPanel({ title, isNested, onBack, children }) {
  return (
    <div className="tablet-embedded-panel">
      <div className="tablet-embedded-panel__header modal-header">
        {isNested ? (
          <button
            type="button"
            className="tablet-embedded-panel__back modal-close-button"
            onClick={onBack}
            aria-label="Back"
          >
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
              <path d="M15 18l-6-6 6-6" />
            </svg>
          </button>
        ) : (
          <span className="tablet-embedded-panel__header-spacer" aria-hidden />
        )}
        <h2 className="modal-title tablet-embedded-panel__title">{title}</h2>
        <span className="tablet-embedded-panel__header-spacer" aria-hidden />
      </div>
      <div className="modal-content tablet-embedded-panel__body">{children}</div>
    </div>
  );
}
