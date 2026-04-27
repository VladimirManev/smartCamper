import { useState, useRef, useEffect } from "react";

/**
 * CustomDropdown Component
 * Custom dropdown component that matches app design and prevents iOS zoom issues
 * 
 * @param {Object} props - Component props
 * @param {string} props.value - Currently selected value
 * @param {Function} props.onChange - Change handler (receives new value)
 * @param {Array} props.options - Array of options: [{ value, label }] or [string]
 * @param {string} props.placeholder - Placeholder text (optional)
 * @param {string} props.className - Additional CSS classes (optional)
 * @param {boolean} props.disabled - Whether dropdown is disabled (optional)
 * @param {"bottom"|"top"} props.menuPlacement - Menu opens below (default) or above the button
 */
export const CustomDropdown = ({
  value,
  onChange,
  options = [],
  placeholder = "Select...",
  className = "",
  disabled = false,
  menuPlacement = "bottom",
}) => {
  const [isOpen, setIsOpen] = useState(false);
  const dropdownRef = useRef(null);
  const buttonRef = useRef(null);
  const pointerTriggeredToggleRef = useRef(false);
  const pointerTriggeredSelectRef = useRef(false);

  // Close dropdown when clicking outside
  useEffect(() => {
    const handleClickOutside = (event) => {
      if (
        dropdownRef.current &&
        !dropdownRef.current.contains(event.target) &&
        buttonRef.current &&
        !buttonRef.current.contains(event.target)
      ) {
        setIsOpen(false);
      }
    };

    if (isOpen) {
      document.addEventListener("mousedown", handleClickOutside);
      document.addEventListener("touchstart", handleClickOutside);
    }

    return () => {
      document.removeEventListener("mousedown", handleClickOutside);
      document.removeEventListener("touchstart", handleClickOutside);
    };
  }, [isOpen]);

  // Close dropdown on escape key
  useEffect(() => {
    const handleEscape = (event) => {
      if (event.key === "Escape" && isOpen) {
        setIsOpen(false);
      }
    };

    if (isOpen) {
      document.addEventListener("keydown", handleEscape);
    }

    return () => {
      document.removeEventListener("keydown", handleEscape);
    };
  }, [isOpen]);

  // Normalize options: handle both [{ value, label }] and [string] formats
  const normalizedOptions = options.map((option) => {
    if (typeof option === "string") {
      return { value: option, label: option };
    }
    return option;
  });

  // Find selected option label
  const selectedOption = normalizedOptions.find((opt) => opt.value === value);
  const displayText = selectedOption ? selectedOption.label : placeholder;

  const handleToggle = () => {
    if (!disabled) {
      setIsOpen(!isOpen);
    }
  };

  const handleSelect = (optionValue) => {
    if (onChange) {
      onChange(optionValue);
    }
    setIsOpen(false);
  };

  return (
    <div className={`custom-dropdown-wrapper ${className}`}>
      <button
        ref={buttonRef}
        type="button"
        className={`custom-dropdown-button ${disabled ? "disabled" : ""} ${isOpen ? "open" : ""}`}
        style={{ touchAction: "manipulation" }}
        onPointerUp={(e) => {
          if (disabled) return;
          if (e.pointerType === "touch" || e.pointerType === "pen") {
            pointerTriggeredToggleRef.current = true;
            handleToggle();
          }
        }}
        onClick={() => {
          if (pointerTriggeredToggleRef.current) {
            pointerTriggeredToggleRef.current = false;
            return;
          }
          handleToggle();
        }}
        disabled={disabled}
        aria-haspopup="listbox"
        aria-expanded={isOpen}
      >
        <span className="custom-dropdown-text">{displayText}</span>
        <svg
          className={`custom-dropdown-arrow ${isOpen ? "open" : ""}`}
          width="12"
          height="12"
          viewBox="0 0 12 12"
          fill="none"
        >
          <path
            d="M6 9L1 4h10z"
            fill="var(--color-accent-blue)"
            stroke="var(--color-accent-blue)"
            strokeWidth="0.5"
          />
        </svg>
      </button>

      {isOpen && (
        <div
          className={`custom-dropdown-menu${
            menuPlacement === "top" ? " custom-dropdown-menu--top" : ""
          }`}
          ref={dropdownRef}
        >
          {normalizedOptions.map((option) => (
            <button
              key={option.value}
              type="button"
              className={`custom-dropdown-option ${
                option.value === value ? "selected" : ""
              }`}
              style={{ touchAction: "manipulation" }}
              onPointerUp={(e) => {
                if (disabled) return;
                if (e.pointerType === "touch" || e.pointerType === "pen") {
                  pointerTriggeredSelectRef.current = true;
                  handleSelect(option.value);
                }
              }}
              onClick={() => {
                if (pointerTriggeredSelectRef.current) {
                  pointerTriggeredSelectRef.current = false;
                  return;
                }
                handleSelect(option.value);
              }}
            >
              {option.label}
            </button>
          ))}
        </div>
      )}
    </div>
  );
};
