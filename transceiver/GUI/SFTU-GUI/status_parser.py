"""
Status message parser for the SFTU GUI application.
Handles parsing of incoming status messages and extracting device information.
"""

from typing import Dict, Optional, NamedTuple
from dataclasses import dataclass


@dataclass
class DeviceStatus:
    """Data class representing device status information."""
    device_id: str
    rssi: str = "N/A"
    battery_voltage: str = ""
    mode: str = ""
    status: str = ""


class StatusParser:
    """Parses incoming status messages from devices."""
    
    # Known device IDs
    DEVICE_SFTU = "SFTU"
    DEVICE_TRANSCEIVER = "transceiver"
    
    def __init__(self):
        """Initialize the status parser."""
        pass
    
    def parse_status_line(self, line: str) -> Optional[DeviceStatus]:
        """
        Parse a status line and extract device information.
        
        Args:
            line: Raw status line from device
            
        Returns:
            DeviceStatus object if parsing successful, None otherwise
        """
        if not line.startswith("status ID:"):
            return None
        
        # Parse key-value pairs from the status line
        parsed_data = self._parse_key_value_pairs(line)
        
        # Extract device ID
        device_id = parsed_data.get("ID", "")
        if not device_id:
            return None
        
        # Create DeviceStatus object
        return DeviceStatus(
            device_id=device_id,
            rssi=parsed_data.get("RSSI", "N/A"),
            battery_voltage=parsed_data.get("battVoltage", 
                                         parsed_data.get("batteryLevel", "")),
            mode=parsed_data.get("mode", ""),
            status=parsed_data.get("status", "")
        )
    
    def _parse_key_value_pairs(self, line: str) -> Dict[str, str]:
        """
        Parse key:value pairs from a line.
        
        Args:
            line: Line containing key:value pairs separated by spaces
            
        Returns:
            Dictionary of key-value pairs
        """
        parsed = {}
        tokens = line.strip().split()
        
        for token in tokens:
            if ':' in token:
                key, value = token.split(':', 1)
                parsed[key] = value
        
        return parsed
    
    def is_known_device(self, device_id: str) -> bool:
        """
        Check if device ID is a known device.
        
        Args:
            device_id: Device identifier
            
        Returns:
            True if device is known, False otherwise
        """
        return device_id in [self.DEVICE_SFTU, self.DEVICE_TRANSCEIVER]


class StatusManager:
    """Manages device status information and updates."""
    
    def __init__(self, rssi_update_callback=None):
        """
        Initialize status manager.
        
        Args:
            rssi_update_callback: Callback function for RSSI updates
                                 Should accept (device_id, rssi_value)
        """
        self.parser = StatusParser()
        self.device_statuses: Dict[str, DeviceStatus] = {}
        self.rssi_update_callback = rssi_update_callback
    
    def process_status_line(self, line: str) -> Optional[DeviceStatus]:
        """
        Process an incoming status line and update device information.
        
        Args:
            line: Raw status line from device
            
        Returns:
            DeviceStatus if processed successfully, None otherwise
        """
        device_status = self.parser.parse_status_line(line)
        
        if device_status is None:
            return None
        
        # Store the status
        self.device_statuses[device_status.device_id] = device_status
        
        # Update RSSI displays if callback is provided
        if self.rssi_update_callback and device_status.rssi != "N/A":
            self.rssi_update_callback(device_status.device_id, device_status.rssi)
        
        return device_status
    
    def get_device_status(self, device_id: str) -> Optional[DeviceStatus]:
        """
        Get status for a specific device.
        
        Args:
            device_id: Device identifier
            
        Returns:
            DeviceStatus if available, None otherwise
        """
        return self.device_statuses.get(device_id)
    
    def get_all_statuses(self) -> Dict[str, DeviceStatus]:
        """
        Get all stored device statuses.
        
        Returns:
            Dictionary of device_id -> DeviceStatus
        """
        return self.device_statuses.copy()
    
    def clear_statuses(self) -> None:
        """Clear all stored device statuses."""
        self.device_statuses.clear()
