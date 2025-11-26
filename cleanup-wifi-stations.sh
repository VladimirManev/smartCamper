#!/bin/bash
# Cleanup inactive WiFi stations script
# Removes stations that don't respond to ping (not actually connected)

# Check each connected device
iw dev wlan0 station dump 2>/dev/null | grep "Station" | awk '{print $2}' | while read MAC; do
    if [ ! -z "$MAC" ]; then
        # Get device information
        STATION_INFO=$(iw dev wlan0 station get "$MAC" 2>/dev/null)
        
        # Check if device has IP address in DHCP leases
        DHCP_LEASE=$(cat /var/lib/misc/dnsmasq.leases 2>/dev/null | grep -i "$MAC")
        
        if [ -z "$DHCP_LEASE" ]; then
            # No DHCP lease - definitely not connected, clean it up
            echo "$(date '+%H:%M:%S'): Cleaning up station without DHCP lease: $MAC"
            sudo iw dev wlan0 station del "$MAC" 2>/dev/null
        else
            # Extract IP address from DHCP lease (format: timestamp mac ip hostname)
            IP_ADDRESS=$(echo "$DHCP_LEASE" | awk '{print $3}')
            
            if [ ! -z "$IP_ADDRESS" ]; then
                # Ping the device (1 ping, 1 second timeout)
                if ping -c 1 -W 1 "$IP_ADDRESS" > /dev/null 2>&1; then
                    # Device responds to ping - keep it
                    echo "$(date '+%H:%M:%S'): Keeping station that responds to ping: $MAC ($IP_ADDRESS)"
                else
                    # Device doesn't respond to ping - clean it up
                    echo "$(date '+%H:%M:%S'): Cleaning up station that doesn't respond to ping: $MAC ($IP_ADDRESS)"
                    sudo iw dev wlan0 station del "$MAC" 2>/dev/null
                fi
            else
                # No IP address in lease - clean it up
                echo "$(date '+%H:%M:%S'): Cleaning up station with invalid lease: $MAC"
                sudo iw dev wlan0 station del "$MAC" 2>/dev/null
            fi
        fi
    fi
done
