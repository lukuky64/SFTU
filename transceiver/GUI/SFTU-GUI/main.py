"""
SFTU GUI Application - Main Entry Point

A PySide6-based GUI application for communicating with SFTU (Short Form Transceiver Unit) devices.
This application provides a user interface for serial communication, device monitoring, and status display.
"""

import sys
from PySide6.QtCore import QObject
from PySide6.QtWidgets import QApplication, QMainWindow

# Import our custom modules
from ui_manager import UIManager
from console_widgets import ConsoleManager
from serial_handler import SerialPort
from status_parser import StatusManager


class SFTUApplication(QMainWindow):
    """Main application window for the SFTU GUI."""
    
    def __init__(self, parent=None):
        """Initialize the SFTU application."""
        super().__init__(parent)
        
        # Initialize managers
        self.ui_manager = UIManager(self)
        self.console_manager = ConsoleManager(self.ui_manager.get_ui().consoleOutput)
        self.serial_port = SerialPort()
        self.status_manager = StatusManager(self._update_rssi_display)
        
        # Track connection state
        self.is_connected = False
        
        self._setup_connections()
        self._setup_ui_handlers()
    
    def _setup_connections(self) -> None:
        """Set up signal connections between components."""
        # Serial port signals
        self.serial_port.data_received.connect(self._handle_incoming_data)
        self.serial_port.connection_status_changed.connect(self._handle_connection_status)
        self.serial_port.error_occurred.connect(self._handle_serial_error)
        
        # Console input signal
        console_input = self.ui_manager.get_console_input()
        if console_input:
            console_input.returnPressed.connect(self._send_command)
    
    def _setup_ui_handlers(self) -> None:
        """Set up UI event handlers."""
        ui = self.ui_manager.get_ui()
        
        # Port selection and connection
        self.ui_manager.setup_port_combo_box(self._refresh_ports)
        ui.comboBox.currentTextChanged.connect(self._port_selection_changed)
        ui.connectButton.clicked.connect(self._toggle_connection)
        
        # Console controls
        ui.clearConsole.clicked.connect(self.console_manager.clear)
    
    def _refresh_ports(self) -> None:
        """Refresh the list of available serial ports."""
        self.ui_manager.refresh_port_list()
    
    def _port_selection_changed(self, text: str) -> None:
        """
        Handle port selection changes.
        
        Args:
            text: Selected port text
        """
        # Enable connect button only if valid port is selected
        valid_port = text.strip() and "n/a" not in text.lower()
        self.ui_manager.set_connect_button_enabled(valid_port and not self.is_connected)
    
    def _toggle_connection(self) -> None:
        """Toggle serial port connection."""
        if self.is_connected:
            self._disconnect()
        else:
            self._connect()
    
    def _connect(self) -> None:
        """Connect to the selected serial port."""
        port_name = self.ui_manager.get_selected_port()
        if port_name:
            success = self.serial_port.connect_to_port(port_name)
            if success:
                self.console_manager.append_message(f"✅ Connected to {port_name}", "system")
    
    def _disconnect(self) -> None:
        """Disconnect from the current serial port."""
        self.serial_port.disconnect()
        self.console_manager.append_message("🔌 Disconnected", "system")
    
    def _send_command(self) -> None:
        """Send command from console input to the connected device."""
        command = self.ui_manager.get_console_input_text()
        
        if not command:
            return
        
        if self.serial_port.send_command(command):
            self.console_manager.append_message(f">> {command}", "user")
            self.ui_manager.clear_console_input()
        else:
            self.console_manager.append_message("⚠️ Not connected to any device.", "system")
    
    def _handle_incoming_data(self, data: str) -> None:
        """
        Handle incoming data from serial port.
        
        Args:
            data: Received data string
        """
        # Display in console
        self.console_manager.append_message(f"<< {data}", "incoming")
        
        # Process status messages
        self.status_manager.process_status_line(data)
    
    def _handle_connection_status(self, connected: bool, message: str) -> None:
        """
        Handle connection status changes.
        
        Args:
            connected: True if connected, False if disconnected
            message: Status message
        """
        self.is_connected = connected
        self.ui_manager.set_connection_state(connected)
        
        # Update connect button availability based on port selection
        if not connected:
            current_text = self.ui_manager.get_ui().comboBox.currentText()
            self._port_selection_changed(current_text)
    
    def _handle_serial_error(self, error_message: str) -> None:
        """
        Handle serial communication errors.
        
        Args:
            error_message: Error description
        """
        self.console_manager.append_message(f"❌ {error_message}", "system")
    
    def _update_rssi_display(self, device_id: str, rssi_value: str) -> None:
        """
        Update RSSI display for a device.
        
        Args:
            device_id: Device identifier
            rssi_value: RSSI value to display
        """
        self.ui_manager.update_rssi_display(device_id, rssi_value)


def main():
    """Main entry point for the SFTU GUI application."""
    app = QApplication(sys.argv)
    
    # Create and show the main window
    window = SFTUApplication()
    window.show()
    
    # Start the application event loop
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
