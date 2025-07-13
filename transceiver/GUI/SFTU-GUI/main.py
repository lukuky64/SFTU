import sys
from PySide6.QtCore import QObject
from PySide6.QtWidgets import QApplication, QMainWindow

# Import our custom modules
from ui_manager import UIManager
from console_widgets import ConsoleManager
from serial_handler import SerialPort
from status_parser import StatusManager

# Import CommandID from parsed header file
from command_parser import CommandID


class SystemMessageManager:
    """Manages system messages by showing only the latest message."""
    
    def __init__(self, text_widget):
        """Initialize with a QTextEdit widget."""
        self.text_widget = text_widget
        
    def append_message(self, text: str, msg_type: str = "system") -> None:
        """
        Display a message, replacing any previous message.
        
        Args:
            text: The message text to display
            msg_type: Type of message (currently unused, for compatibility)
        """
        # Simply set the text, replacing any existing content
        self.text_widget.setPlainText(text)


class SFTUApplication(QMainWindow):
    """Main application window for the SFTU GUI."""
    
    def __init__(self, parent=None):
        """Initialize the SFTU application."""
        super().__init__(parent)
        
        # Initialize managers
        self.ui_manager = UIManager(self)
        self.console_manager = ConsoleManager(self.ui_manager.get_ui().consoleOutput)
        self.system_msgs = SystemMessageManager(self.ui_manager.get_ui().system_msgs)
        self.serial_port = SerialPort()
        self.status_manager = StatusManager(self._update_device_status_display)
        
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
        
        # LoRa updates
        self.ui_manager.setup_frequency_button(self._update_frequency)
        self.ui_manager.setup_bandwidth_button(self._update_bandwidth)
        self.ui_manager.setup_spreadingFactor_button(self._update_spreadingFactor)
        self.ui_manager.setup_gain_button(self._update_gain)

        # Output states - track state of all 8 outputs
        self.output_states = [False] * 8  # List to track states of outputs 1-8

        # Output control - setup all 8 output buttons
        self.ui_manager.setup_output_buttons(self._update_output)
    
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
            self.ui_manager.set_update_buttons_enabled(False)
        else:
            self._connect()
            self.ui_manager.set_update_buttons_enabled(True)
    
    def _connect(self) -> None:
        """Connect to the selected serial port."""
        port_name = self.ui_manager.get_selected_port()
        if port_name:
            success = self.serial_port.connect_to_port(port_name)
            if success:
                self.system_msgs.append_message(f"✅ Connected to {port_name}", "system")
    
    def _disconnect(self) -> None:
        """Disconnect from the current serial port."""
        self.serial_port.disconnect()
        self.system_msgs.append_message("🔌 Disconnected", "system")
    
    def _send_command(self) -> None:
        """Send command from console input to the connected device."""
        command = self.ui_manager.get_console_input_text()
        
        if not command:
            return
        
        if self.serial_port.send_command(command):
            self.console_manager.append_message(f">> {command}", "user")
            self.ui_manager.clear_console_input()
        else:
            self.system_msgs.append_message("⚠️ Not connected to any device.", "system")
    
    def _handle_incoming_data(self, data: str) -> None:
        """
        Handle incoming data from serial port.
        
        Args:
            data: Received data string
        """
        # Display in console
        self.console_manager.append_message(f"<< {data}", "incoming")
        
        # # Debug: Log if this looks like a status message
        # if "status ID:" in data or "ID:" in data:
        #     self.console_manager.append_message(f"🔍 Debug: Detected potential status message: {data}", "system")
        
        # Process status messages
        status_result = self.status_manager.process_status_line(data)
        
        # # Debug: Log status processing result
        # if status_result:
        #     self.console_manager.append_message(f"✅ Debug: Status parsed successfully for {status_result.device_id}", "system")
        # elif "status ID:" in data:
        #     self.console_manager.append_message(f"❌ Debug: Status parsing failed for: {data}", "system")
    
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
        self.system_msgs.append_message(f"❌ {error_message}", "system")
    
    def _update_device_status_display(self, device_id: str, device_status) -> None:
        """
        Update device status display.
        
        Args:
            device_id: Device identifier
            device_status: DeviceStatus object with device information
        """
        # Debug: Log status display updates
        # self.console_manager.append_message(f"📊 Debug: Updating status display for {device_id}: {device_status}", "system")
        
        self.ui_manager.update_device_status_display(device_id, device_status)
    
    def _update_frequency(self) -> None:
        """Handle frequency update button press."""

        # Get and validate frequency
        freq_str = self.ui_manager.get_frequency_value()
        is_valid, error_message, freq_value = self.ui_manager.validate_value(freq_str, 900.0, 1000.0)
        
        if not is_valid:
            self.system_msgs.append_message(f"❌ {error_message}", "system")
            return
        
        command = f"{CommandID.CMD_UPDATE_FREQMHZ} {freq_value}"
        self._update_CMD(command)

    def _update_bandwidth(self) -> None:
        """Handle bandwidth update button press."""

        # Get and validate bandwidth
        bandw_str = self.ui_manager.get_bandwidth_value()
        is_valid, error_message, bw_value = self.ui_manager.validate_value(bandw_str, 5.0, 510.0)
        
        if not is_valid:
            self.system_msgs.append_message(f"❌ {error_message}", "system")
            return
        
        command = f"{CommandID.CMD_UPDATE_BW} {bw_value}"
        self._update_CMD(command)

    def _update_spreadingFactor(self) -> None:
        """Handle spreading factor update button press."""

        # Get and validate spreading factor
        sf_str = self.ui_manager.get_spreadingFactor_value()
        is_valid, error_message, sf_value = self.ui_manager.validate_value(sf_str, 7.0, 12.0)
        
        if not is_valid:
            self.system_msgs.append_message(f"❌ {error_message}", "system")
            return
        
        command = f"{CommandID.CMD_UPDATE_SF} {sf_value}"
        self._update_CMD(command)

    def _update_gain(self) -> None:
        """Handle gain update button press."""

        # Get and validate gain
        gain_str = self.ui_manager.get_gain_value()
        is_valid, error_message, gain_value = self.ui_manager.validate_value(gain_str, 0, 22)
        
        if not is_valid:
            self.system_msgs.append_message(f"❌ {error_message}", "system")
            return
        
        # Send frequency update command
        command = f"{CommandID.CMD_UPDATE_GAIN} {gain_value}"
        self._update_CMD(command)

    def _update_output(self, button_index: int) -> None:
        """Handle output button press for any of the 8 output buttons."""
        # Toggle the state for this button (convert to 0-based index)
        array_index = button_index - 1
        self.output_states[array_index] = not self.output_states[array_index]
        
        # 1 for ON, 0 for OFF
        _state = "1" if self.output_states[array_index] else "0"

        # Change color of the button
        button = self.ui_manager.get_output_button(button_index)
        if button:
            if self.output_states[array_index]:
                button.setStyleSheet("background-color: #3df546; color: white;")  # Green for ON
            else:
                button.setStyleSheet("background-color: #f53d3d; color: white;")  # Red for OFF

        # Send command
        command = f"{CommandID.CMD_SET_OUTPUT} {float(f'{button_index}.{_state}')}"
        self._update_CMD(command)

    def _update_CMD(self, command) -> None:
        """Handle frequency update button press."""
        # Check if serial is connected
        if not self.is_connected:
            self.system_msgs.append_message("⚠️ Not connected to any device.", "system")
            return
        
        if self.serial_port.send_command(command):
            self.system_msgs.append_message(f"✅ Update sent: >> {command} <<", "system")
        else:
            self.system_msgs.append_message("❌ Failed to send frequency update command", "system")
    
    def get_output_states(self) -> list:
        """Get current states of all output buttons."""
        return self.output_states.copy()
    
    def reset_all_outputs(self) -> None:
        """Reset all outputs to OFF state."""
        for i in range(1, 9):
            if self.output_states[i-1]:  # Only reset if currently ON
                self._update_output(i)
    


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
