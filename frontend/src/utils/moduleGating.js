/**
 * Module UI flags — honours FEATURE_FLAGS.
 */

import { FEATURE_FLAGS } from "../config/featureFlags";

export function isModuleStatusIconsEnabled() {
  return FEATURE_FLAGS.moduleStatusIcons;
}
