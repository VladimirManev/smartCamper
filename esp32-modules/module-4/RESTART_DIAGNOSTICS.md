# Module 4 Restart Diagnostics

## Problem
Module 4 restarts during the night, causing all dampers to open (default behavior on boot).

## Added Diagnostics

### 1. Reset Reason Detection
On every boot, the module now prints:
- **Reset reason** (BROWNOUT, WATCHDOG, PANIC, etc.)
- **Memory status** (free heap, min free heap, largest free block)
- **Chip information** (model, CPU frequency)

### 2. Periodic Memory Monitoring
Every 5 minutes, the module logs:
- Free heap memory
- Minimum free heap (lowest point since boot)
- Largest free block
- Uptime
- Warning if heap < 20KB

### 3. Watchdog Protection
Added `yield()` in loop() to prevent task watchdog timeout.

## How to Diagnose

### Step 1: Check Serial Output
After the restart, check the serial monitor for:
```
🔍 === RESET DIAGNOSTICS ===
Reset Reason: [REASON]
```

**Common reset reasons:**
- `BROWNOUT` - Low voltage (most likely cause)
- `TASK_WDT` - Watchdog timeout (loop() blocked too long)
- `PANIC` - Software exception/crash
- `INT_WDT` - Interrupt watchdog timeout

### Step 2: Monitor Memory
Watch for memory leaks:
- If `Min Free Heap` decreases over time → memory leak
- If `Free Heap` drops below 20KB → potential problem

### Step 3: Check Power Supply
If reset reason is `BROWNOUT`:
- Check power supply voltage (should be 5V stable)
- Check for loose connections
- Check if other devices on same power line cause voltage drops
- Consider adding capacitor for power smoothing

### Step 4: Check for Blocking Code
If reset reason is `TASK_WDT`:
- Look for blocking `delay()` calls in loop()
- Check if WiFi/MQTT reconnection blocks too long
- Verify `yield()` is called regularly

### Step 5: Check for Exceptions
If reset reason is `PANIC`:
- Look for array bounds violations
- Check for null pointer dereferences
- Verify JSON parsing doesn't crash on invalid data

## Potential Causes

### Most Likely: Brownout Detection
- **Symptom**: Reset reason = `BROWNOUT`
- **Cause**: Voltage drops below threshold (typically 2.8V)
- **Solution**: 
  - Check power supply quality
  - Add capacitor (1000µF) near ESP32
  - Check for loose connections
  - Verify power supply can handle current draw

### Watchdog Timeout
- **Symptom**: Reset reason = `TASK_WDT` or `INT_WDT`
- **Cause**: Loop() blocked for >5 seconds
- **Solution**: 
  - Already added `yield()` in loop()
  - Check for blocking operations
  - Verify all delays are non-blocking

### Memory Issues
- **Symptom**: Reset reason = `PANIC`, or memory logs show leaks
- **Cause**: Heap corruption, stack overflow, memory leak
- **Solution**:
  - Monitor memory logs
  - Check for array bounds violations
  - Verify dynamic memory allocation/deallocation

### WiFi/MQTT Issues
- **Symptom**: Frequent reconnections, then restart
- **Cause**: Network issues causing crashes
- **Solution**:
  - Check WiFi signal strength
  - Verify MQTT broker is stable
  - Check for network-related exceptions

## Next Steps

1. **Deploy the diagnostic code** to the module
2. **Monitor serial output** for several nights
3. **Record reset reasons** when restarts occur
4. **Check memory trends** over time
5. **Based on findings**, implement specific fixes

## Additional Recommendations

### For Brownout Protection
If brownout is the cause, consider:
- Adding hardware brownout detection disable (not recommended)
- Improving power supply quality
- Adding power smoothing capacitors
- Using separate power supply for ESP32

### For Watchdog Protection
Already implemented:
- `yield()` in main loop()
- Non-blocking delays in NetworkManager

### For Memory Protection
- Monitor memory logs
- Consider reducing JSON buffer sizes if needed
- Check for memory leaks in long-running code

## Testing

To test the diagnostics:
1. Upload the updated code
2. Monitor serial output for 24+ hours
3. Check reset reason on next restart
4. Review memory logs for trends
5. Implement fixes based on findings
