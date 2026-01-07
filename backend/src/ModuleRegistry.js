/**
 * Module Registry
 * Tracks heartbeat status for all ESP32 modules
 * Provides centralized module status management
 */

class ModuleRegistry {
  constructor() {
    // Map of moduleId -> { status, lastHeartbeat, metadata }
    this.modules = new Map();
    
    // Heartbeat timeout threshold (20 seconds = 2x heartbeat interval)
    // This allows for 2 missed heartbeats before marking as offline
    this.HEARTBEAT_TIMEOUT_MS = 20000;
    
    // Check interval (2 seconds) - faster detection of offline modules
    this.CHECK_INTERVAL_MS = 2000;
    
    // Interval timer for periodic checks
    this.checkInterval = null;
    
    // Callback for status changes
    this.onStatusChange = null;
  }

  /**
   * Initialize the registry
   * @param {Function} onStatusChange - Callback when module status changes
   */
  initialize(onStatusChange) {
    this.onStatusChange = onStatusChange;
    
    // Start periodic check for missing heartbeats
    this.startPeriodicCheck();
    
    console.log("📊 Module Registry initialized");
  }

  /**
   * Calculate number of signal bars based on RSSI value
   * Same logic as frontend SignalIndicator component
   * @param {number|null} rssi - WiFi RSSI value in dBm
   * @returns {number} Number of bars (0-4)
   */
  getSignalBars(rssi) {
    if (rssi === null || rssi === undefined || rssi === -999) {
      return 0; // No WiFi
    }
    
    if (rssi >= -50) {
      return 4; // Excellent signal (-30 to -50 dBm)
    } else if (rssi >= -60) {
      return 3; // Very good signal (-50 to -60 dBm)
    } else if (rssi >= -70) {
      return 2; // Good signal (-60 to -70 dBm)
    } else if (rssi >= -80) {
      return 1; // Weak signal (-70 to -80 dBm)
    } else {
      return 0; // Very weak signal (below -80 dBm)
    }
  }

  /**
   * Process heartbeat message from module
   * @param {string} moduleId - Module identifier
   * @param {Object} heartbeatData - Heartbeat payload data
   */
  processHeartbeat(moduleId, heartbeatData) {
    const now = Date.now();
    const wasOnline = this.isModuleOnline(moduleId);
    const oldModule = this.modules.get(moduleId);
    const oldRSSI = oldModule?.wifiRSSI;
    const oldBars = oldModule ? this.getSignalBars(oldRSSI) : 0;
    
    // Update or create module entry
    const moduleInfo = {
      status: "online",
      lastHeartbeat: now,
      moduleId: moduleId,
      timestamp: heartbeatData.timestamp || null,
      uptime: heartbeatData.uptime || null,
      wifiRSSI: heartbeatData.wifiRSSI || null,
      metadata: heartbeatData,
    };
    
    this.modules.set(moduleId, moduleInfo);
    
    // Calculate new signal bars count
    const newBars = this.getSignalBars(heartbeatData.wifiRSSI);
    
    // Notify if:
    // 1. Status changed from offline to online, OR
    // 2. Signal bars count changed (RSSI crossed a threshold)
    const barsChanged = oldBars !== newBars;
    if (!wasOnline || barsChanged) {
      this.notifyStatusChange();
    }
  }

  /**
   * Check if module is online
   * @param {string} moduleId - Module identifier
   * @returns {boolean} True if module is online
   */
  isModuleOnline(moduleId) {
    const module = this.modules.get(moduleId);
    if (!module) {
      return false;
    }
    
    const now = Date.now();
    const timeSinceLastHeartbeat = now - module.lastHeartbeat;
    
    return timeSinceLastHeartbeat < this.HEARTBEAT_TIMEOUT_MS;
  }

  /**
   * Get module status
   * @param {string} moduleId - Module identifier
   * @returns {Object|null} Module status object or null if not found
   */
  getModuleStatus(moduleId) {
    const module = this.modules.get(moduleId);
    if (!module) {
      return null;
    }
    
    const isOnline = this.isModuleOnline(moduleId);
    
    return {
      moduleId: moduleId,
      status: isOnline ? "online" : "offline",
      lastSeen: new Date(module.lastHeartbeat).toISOString(),
      lastHeartbeat: module.lastHeartbeat,
      uptime: module.uptime,
      wifiRSSI: module.wifiRSSI,
      metadata: module.metadata,
    };
  }

  /**
   * Get all module statuses
   * @returns {Object} Map of moduleId -> status
   */
  getAllModuleStatuses() {
    const statuses = {};
    
    for (const [moduleId, module] of this.modules.entries()) {
      statuses[moduleId] = this.getModuleStatus(moduleId);
    }
    
    return statuses;
  }

  /**
   * Start periodic check for missing heartbeats
   */
  startPeriodicCheck() {
    if (this.checkInterval) {
      clearInterval(this.checkInterval);
    }
    
    this.checkInterval = setInterval(() => {
      this.checkHeartbeats();
    }, this.CHECK_INTERVAL_MS);
  }

  /**
   * Check all modules for missing heartbeats
   */
  checkHeartbeats() {
    const now = Date.now();
    let statusChanged = false;
    
    for (const [moduleId, module] of this.modules.entries()) {
      const wasOnline = module.status === "online";
      const timeSinceLastHeartbeat = now - module.lastHeartbeat;
      const isOnline = timeSinceLastHeartbeat < this.HEARTBEAT_TIMEOUT_MS;
      
      if (wasOnline !== isOnline) {
        module.status = isOnline ? "online" : "offline";
        statusChanged = true;
      }
    }
    
    if (statusChanged) {
      this.notifyStatusChange();
    }
  }

  /**
   * Notify about status changes
   */
  notifyStatusChange() {
    if (this.onStatusChange) {
      const allStatuses = this.getAllModuleStatuses();
      this.onStatusChange(allStatuses);
    }
  }

  /**
   * Stop periodic check
   */
  stop() {
    if (this.checkInterval) {
      clearInterval(this.checkInterval);
      this.checkInterval = null;
    }
  }

  /**
   * Get registry statistics
   * @returns {Object} Statistics about modules
   */
  getStatistics() {
    const stats = {
      totalModules: this.modules.size,
      onlineModules: 0,
      offlineModules: 0,
    };
    
    for (const [moduleId] of this.modules.entries()) {
      if (this.isModuleOnline(moduleId)) {
        stats.onlineModules++;
      } else {
        stats.offlineModules++;
      }
    }
    
    return stats;
  }
}

module.exports = ModuleRegistry;

