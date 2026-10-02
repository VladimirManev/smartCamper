/**
 * Fixed 2-row metric block for status/detail modals.
 * 1 item → top slot; 2 → both — keeps swipe layouts aligned.
 */

/**
 * @param {Object} props
 * @param {{ label: string, value: string }[]} props.items
 * @param {string} [props.ariaLabel]
 */
export function ModalStatSlots({ items = [], ariaLabel }) {
  const list = Array.isArray(items) ? items.slice(0, 2) : [];
  /** @type {({ label: string, value: string }|null)[]} */
  const slots = [null, null];

  if (list.length === 1) {
    slots[0] = list[0];
  } else if (list.length >= 2) {
    slots[0] = list[0];
    slots[1] = list[1];
  }

  return (
    <div className="modal-stat-slots" aria-label={ariaLabel}>
      {slots.map((item, index) => (
        <div
          key={index}
          className={[
            "modal-stat-slot",
            !item && "modal-stat-slot--empty",
          ]
            .filter(Boolean)
            .join(" ")}
          aria-hidden={!item}
        >
          {item ? (
            <>
              <span className="modal-stat-slot__label">{item.label}</span>
              <span className="modal-stat-slot__value">{item.value}</span>
            </>
          ) : null}
        </div>
      ))}
    </div>
  );
}
