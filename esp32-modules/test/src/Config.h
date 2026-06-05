#ifndef CONFIG_H
#define CONFIG_H

// Victron BLE device credentials from VictronConnect:
// Settings -> Product Info -> Instant Readout via Bluetooth -> Show

#define BLE_SCAN_SECONDS 3

#define VICTRON_DEVICE_COUNT 4

#define VICTRON_DEVICE_0_NAME "SmartShunt"
#define VICTRON_DEVICE_0_MAC "E7:47:43:C9:5D:09"
#define VICTRON_DEVICE_0_KEY "af621f5c41707664f6b5ef35ad0d2b99"

#define VICTRON_DEVICE_1_NAME "Orion"
#define VICTRON_DEVICE_1_MAC "E8:42:AE:38:C1:C6"
#define VICTRON_DEVICE_1_KEY "cefa9834a87e10fc946b39e644d00998"

#define VICTRON_DEVICE_2_NAME "MPPT1"
#define VICTRON_DEVICE_2_MAC "D3:AD:2A:CC:47:8C"
#define VICTRON_DEVICE_2_KEY "da5ba206e2dc1dc7c6d7d5516dc350b0"

#define VICTRON_DEVICE_3_NAME "MPPT2"
#define VICTRON_DEVICE_3_MAC "DC:41:88:BE:96:18"
#define VICTRON_DEVICE_3_KEY "ac0f048ff4ff8c411f2c5eb05bc42a5e"

#endif
