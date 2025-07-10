"""
Serial communication handler for the SFTU GUI application.
Manages serial port connections, data transmission, and port discovery.
"""

import serial
import serial.tools.list_ports
from typing import Optional, List, Callable
from PySide6.QtCore import QObject, QTimer, Signal


class SerialPort(QObject):
    """Handles serial communication with external devices."""
    
    # Signals for communication events
    data_received = Signal(str)  # Emitted when data is received
    connection_status_changed = Signal(bool, str)  # Emitted when connection status changes
    error_occurred = Signal(str)  # Emitted when an error occurs
    
    def __init__(self, baudrate: int = 115200, timeout: float = 0.1):
        """
        Initialize serial port handler.
        
        Args:
            baudrate: Serial communication baud rate
            timeout: Read timeout in seconds
        """
        super().__init__()
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial_connection: Optional[serial.Serial] = None
        self.read_buffer = ""
        
        # Timer for reading data from serial port
        self.read_timer = QTimer()
        self.read_timer.timeout.connect(self._read_data)
        self.read_timer.setInterval(100)  # Read every 100ms
    
    @property
    def is_connected(self) -> bool:
        """Check if serial port is connected."""
        return self.serial_connection is not None and self.serial_connection.is_open
    
    def connect_to_port(self, port_name: str) -> bool:
        """
        Connect to a specific serial port.
        
        Args:
            port_name: Name of the port to connect to
            
        Returns:
            True if connection successful, False otherwise
        """
        try:
            self.serial_connection = serial.Serial(
                port_name, 
                baudrate=self.baudrate, 
                timeout=self.timeout
            )
            self.read_timer.start()
            self.connection_status_changed.emit(True, f"Connected to {port_name}")
            return True
            
        except serial.SerialException as e:
            error_msg = f"Failed to connect to {port_name}: {e}"
            self.error_occurred.emit(error_msg)
            self.connection_status_changed.emit(False, error_msg)
            self.serial_connection = None
            return False
    
    def disconnect(self) -> None:
        """Disconnect from the current serial port."""
        if self.is_connected:
            self.read_timer.stop()
            self.serial_connection.close()
            self.serial_connection = None
            self.connection_status_changed.emit(False, "Disconnected")
    
    def send_command(self, command: str) -> bool:
        """
        Send a command to the connected device.
        
        Args:
            command: Command string to send
            
        Returns:
            True if sent successfully, False otherwise
        """
        if not self.is_connected:
            self.error_occurred.emit("Not connected to any device")
            return False
        
        try:
            self.serial_connection.write((command + '\n').encode())
            return True
        except Exception as e:
            self.error_occurred.emit(f"Failed to send command: {e}")
            return False
    
    def _read_data(self) -> None:
        """Read data from the serial port."""
        if not self.is_connected:
            return

        try:
            # Read available data
            raw_data = self.serial_connection.read(self.serial_connection.in_waiting or 1)
            if not raw_data:
                return

            # Decode and add to buffer
            decoded_data = raw_data.decode(errors='replace')
            self.read_buffer += decoded_data

            # Process complete lines
            while '\n' in self.read_buffer:
                line, self.read_buffer = self.read_buffer.split('\n', 1)
                line = line.strip()
                if line:  # Only emit non-empty lines
                    self.data_received.emit(line)

        except Exception as e:
            self.error_occurred.emit(f"Serial read error: {e}")
            # Attempt to disable update buttons in the UI if possible
            try:
                if hasattr(self.parent(), 'ui_manager'):
                    self.parent().ui_manager.set_update_buttons_enabled(False)
            except Exception:
                pass
            self.disconnect()
            


class PortDiscovery:
    """Utility class for discovering available serial ports."""
    
    @staticmethod
    def get_available_ports() -> List[tuple]:
        """
        Get list of available serial ports.
        
        Returns:
            List of (device_name, description) tuples
        """
        ports = []
        for port in serial.tools.list_ports.comports():
            # Skip ports with "n/a" in description
            if "n/a" not in port.description.lower():
                ports.append((port.device, port.description))
        return ports
    
    @staticmethod
    def format_port_display(device_name: str, description: str) -> str:
        """
        Format port information for display.
        
        Args:
            device_name: Device name (e.g., COM3, /dev/ttyUSB0)
            description: Port description
            
        Returns:
            Formatted string for display
        """
        return f"{device_name} — {description}"
    
    @staticmethod
    def extract_port_name(formatted_port: str) -> str:
        """
        Extract port name from formatted display string.
        
        Args:
            formatted_port: Formatted port string
            
        Returns:
            Port device name
        """
        return formatted_port.split("—")[0].strip()
