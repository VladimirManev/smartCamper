/**
 * DOM edge points and SVG wire geometry for the battery energy diagram.
 */

/** @typedef {'top' | 'bottom' | 'left' | 'right'} WireEdge */

/**
 * @param {DOMRect} rect
 * @param {DOMRect} containerRect
 * @param {WireEdge} edge
 * @param {{ atY?: number, topAnchor?: number, atX?: number }} [opts]
 */
export function getRectEdgePoint(rect, containerRect, edge, opts = {}) {
  const cx = rect.left + rect.width / 2 - containerRect.left;
  const cy = rect.top + rect.height / 2 - containerRect.top;
  const lx = rect.left - containerRect.left;
  const rx = rect.right - containerRect.left;
  const ty = rect.top - containerRect.top;
  const by = rect.bottom - containerRect.top;

  switch (edge) {
    case "top":
      return {
        x:
          opts.atX != null
            ? Math.max(lx, Math.min(rx, opts.atX))
            : opts.topAnchor != null
              ? rect.left + rect.width * opts.topAnchor - containerRect.left
              : cx,
        y: ty,
      };
    case "bottom":
      return {
        x:
          opts.atX != null
            ? Math.max(lx, Math.min(rx, opts.atX))
            : cx,
        y: by,
      };
    case "left":
      return { x: lx, y: opts.atY != null ? opts.atY : cy };
    case "right":
      return { x: rx, y: opts.atY != null ? opts.atY : cy };
    default:
      return { x: cx, y: cy };
  }
}

/**
 * @param {HTMLElement|null} batterySlotRef
 */
function getBatteryShellRect(batterySlotRef) {
  const shell = batterySlotRef?.querySelector(".battery-diagram-center__shell");
  if (!shell) return null;
  return shell.getBoundingClientRect();
}

/**
 * @param {HTMLElement|null} batterySlotRef
 * @param {DOMRect} containerRect
 * @param {WireEdge} edge
 * @param {{ atY?: number, topAnchor?: number, atX?: number }} [opts]
 */
function getBatteryShellPoint(batterySlotRef, containerRect, edge, opts = {}) {
  const rect = getBatteryShellRect(batterySlotRef);
  if (!rect) return null;

  const ty = rect.top - containerRect.top;
  const by = rect.bottom - containerRect.top;
  const point = getRectEdgePoint(rect, containerRect, edge, opts);

  if (opts.atY != null) {
    point.y = Math.max(ty, Math.min(by, opts.atY));
  }

  return point;
}

/**
 * @param {string} nodeId
 * @param {WireEdge} edge
 * @param {Record<string, HTMLElement>} slotRefs
 * @param {HTMLElement|null} batterySlotRef
 * @param {DOMRect} containerRect
 * @param {{ atY?: number, topAnchor?: number, atX?: number }} [opts]
 */
export function getWireNodePoint(
  nodeId,
  edge,
  slotRefs,
  batterySlotRef,
  containerRect,
  opts = {}
) {
  if (nodeId === "battery") {
    return getBatteryShellPoint(batterySlotRef, containerRect, edge, opts);
  }

  const card = slotRefs[nodeId]?.querySelector(".battery-node-card");
  if (!card) return null;
  return getRectEdgePoint(card.getBoundingClientRect(), containerRect, edge, opts);
}

function withLineLabel(shape, x1, y1, x2, y2) {
  const mx = (x1 + x2) / 2;
  const my = (y1 + y2) / 2;
  const dx = Math.abs(x2 - x1);
  const dy = Math.abs(y2 - y1);
  let labelX = mx;
  let labelY = my;

  if (dx >= dy) {
    labelY -= 9;
  } else {
    labelX += 26;
  }

  return { ...shape, labelX, labelY, labelVertical: dy > dx };
}

/**
 * @param {{ x: number, y: number }} start
 * @param {{ x: number, y: number }} end
 */
export function buildStraightWire(start, end) {
  const shape = {
    type: "line",
    x1: start.x,
    y1: start.y,
    x2: end.x,
    y2: end.y,
  };
  return withLineLabel(shape, start.x, start.y, end.x, end.y);
}

/**
 * Rounded polyline (fillet at inner vertices).
 * @param {Array<{ x: number, y: number }>} pts
 * @param {number} radius
 */
export function buildFilletedPath(pts, radius = 6) {
  if (pts.length < 2) return "";
  if (pts.length === 2) {
    return `M ${pts[0].x} ${pts[0].y} L ${pts[1].x} ${pts[1].y}`;
  }

  const parts = [`M ${pts[0].x} ${pts[0].y}`];

  for (let i = 1; i < pts.length - 1; i++) {
    const prev = pts[i - 1];
    const curr = pts[i];
    const next = pts[i + 1];

    const v1x = curr.x - prev.x;
    const v1y = curr.y - prev.y;
    const v2x = next.x - curr.x;
    const v2y = next.y - curr.y;
    const len1 = Math.hypot(v1x, v1y);
    const len2 = Math.hypot(v2x, v2y);
    if (len1 < 0.001 || len2 < 0.001) continue;

    const r = Math.min(radius, len1 / 2, len2 / 2);
    const p1 = {
      x: curr.x - (v1x / len1) * r,
      y: curr.y - (v1y / len1) * r,
    };
    const p2 = {
      x: curr.x + (v2x / len2) * r,
      y: curr.y + (v2y / len2) * r,
    };

    parts.push(`L ${p1.x} ${p1.y}`);
    parts.push(`Q ${curr.x} ${curr.y} ${p2.x} ${p2.y}`);
  }

  const last = pts[pts.length - 1];
  parts.push(`L ${last.x} ${last.y}`);
  return parts.join(" ");
}

/**
 * Horizontal segment from source side to battery shell at source Y.
 */
function buildHorizontalToBatteryWire(start, batterySlotRef, containerRect, toEdge) {
  const end = getBatteryShellPoint(batterySlotRef, containerRect, toEdge, {
    atY: start.y,
  });
  if (!end) return null;
  return buildStraightWire(start, end);
}

/**
 * AC Charger → battery top (equal vertical legs, rounded bends).
 */
function buildElbowToTopWire(start, end, elbowYRatio = 0.5, fillet = 12) {
  const midY = start.y + (end.y - start.y) * elbowYRatio;
  const d = buildFilletedPath(
    [start, { x: start.x, y: midY }, { x: end.x, y: midY }, end],
    fillet
  );
  return {
    type: "path",
    d,
    labelX: (start.x + end.x) / 2,
    labelY: midY - 9,
    labelVertical: false,
  };
}

/**
 * @param {import("../config/batterySystemNodes").BATTERY_WIRE_LINKS[number]} link
 * @param {Record<string, HTMLElement>} slotRefs
 * @param {HTMLElement|null} batterySlotRef
 * @param {DOMRect} containerRect
 */
export function buildWireShape(link, slotRefs, batterySlotRef, containerRect) {
  if (link.route === "horizontal") {
    const start = getWireNodePoint(
      link.from,
      link.fromEdge,
      slotRefs,
      batterySlotRef,
      containerRect
    );
    if (!start) return null;
    const line = buildHorizontalToBatteryWire(
      start,
      batterySlotRef,
      containerRect,
      link.toEdge
    );
    return line ? { id: link.id, flow: link.flow, ...line } : null;
  }

  if (link.route === "elbow-to-top") {
    const end = getWireNodePoint(
      link.to,
      link.toEdge,
      slotRefs,
      batterySlotRef,
      containerRect,
      { topAnchor: link.topAnchor ?? 0.34 }
    );
    const start = getWireNodePoint(
      link.from,
      link.fromEdge,
      slotRefs,
      batterySlotRef,
      containerRect
    );
    if (!start || !end) return null;
    return {
      id: link.id,
      flow: link.flow,
      ...buildElbowToTopWire(
        start,
        end,
        link.elbowYRatio ?? 0.5,
        link.filletRadius ?? 12
      ),
    };
  }

  const endOpts = {};
  if (link.toEdge === "top" && link.topAnchor != null) {
    endOpts.topAnchor = link.topAnchor;
  }

  const end = getWireNodePoint(
    link.to,
    link.toEdge,
    slotRefs,
    batterySlotRef,
    containerRect,
    endOpts
  );
  if (!end) return null;

  const startOpts = {};
  if (link.alignVertical && link.fromEdge === "bottom") {
    startOpts.atX = end.x;
  }

  const start = getWireNodePoint(
    link.from,
    link.fromEdge,
    slotRefs,
    batterySlotRef,
    containerRect,
    startOpts
  );
  if (!start) return null;

  return { id: link.id, flow: link.flow, ...buildStraightWire(start, end) };
}
