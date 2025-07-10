"""
UI manager for the SFTU GUI application.
Handles UI setup, widget configuration, and UI-related operations.
"""

from typing import Optional
from PySide6.QtWidgets import QComboBox, QMainWindow
from console_widgets import ConsoleInput
from serial_handler import PortDiscovery
from ui_form import Ui_main


class UIManager:
    """Manages UI setup and configuration."""
    
    def __init__(self, main_window: QMainWindow):
        """
        Initialize UI manager.
        
        Args:
            main_window: The main window instance
        """
        self.main_window = main_window
        self.ui = Ui_main()
        self.ui.setupUi(main_window)
        self.console_input: Optional[ConsoleInput] = None
        
        self._setup_custom_widgets()
        self._setup_initial_state()
    
    def _setup_custom_widgets(self) -> None:
        """Set up custom widgets to replace designer widgets."""
        self._replace_console_input()
    
    def _replace_console_input(self) -> None:
        """Replace the designer's console input with custom ConsoleInput widget."""
        # Create custom console input
        self.console_input = ConsoleInput(self.main_window)
        self.console_input.setObjectName("consoleSend")
        self.console_input.setGeometry(self.ui.consoleSend.geometry())
        self.console_input.setFont(self.ui.consoleSend.font())
        
        # Replace in layout or parent
        parent = self.ui.consoleSend.parent()
        layout = parent.layout()
        
        if layout:
            # Replace in layout
            index = layout.indexOf(self.ui.consoleSend)
            layout.removeWidget(self.ui.consoleSend)
            self.ui.consoleSend.deleteLater()
            layout.insertWidget(index, self.console_input)
        else:
            # Manual replacement
            self.console_input.setParent(parent)
            self.ui.consoleSend.hide()
            self.ui.consoleSend.deleteLater()
            self.console_input.show()
    
    def _setup_initial_state(self) -> None:
        """Set up initial UI state."""
        self.ui.connectButton.setEnabled(False)
        if self.console_input:
            self.console_input.setEnabled(False)
        
        # Initialize device status displays
        self.initialize_device_status_displays()
        
        # Initialize output button states
        self.initialize_output_button_states()
    
    def setup_port_combo_box(self, refresh_callback) -> None:
        """
        Set up the port combo box with refresh functionality.
        
        Args:
            refresh_callback: Callback to call when ports need refreshing
        """
        # Monkey patch the showPopup method to refresh ports
        original_popup = self.ui.comboBox.showPopup
        
        def new_popup():
            refresh_callback()
            original_popup()
        
        self.ui.comboBox.showPopup = new_popup
    
    def setup_frequency_button(self, update_callback) -> None:
        """
        Set up the frequency update button.
        
        Args:
            update_callback: Callback to call when frequency update is requested
        """
        self.ui.updateFreqButton.clicked.connect(update_callback)

    def setup_bandwidth_button(self, update_callback) -> None:
        """
        Set up the bandwidth update button.
        
        Args:
            update_callback: Callback to call when bandwidth update is requested
        """
        self.ui.updateBandwidthButton.clicked.connect(update_callback)
    
    def setup_spreadingFactor_button(self, update_callback) -> None:
        """
        Set up the spreading factor update button.
        
        Args:
            update_callback: Callback to call when spreading factor update is requested
        """
        self.ui.updateSpreadButton.clicked.connect(update_callback)

    def setup_gain_button(self, update_callback) -> None:
        """
        Set up the gain update button.
        
        Args:
            update_callback: Callback to call when gain update is requested
        """
        self.ui.updateGainButton.clicked.connect(update_callback)

    def setup_output_button1(self, update_callback) -> None:
        """
        Set up the output control button.
        
        Args:
            update_callback: Callback to call when output control is requested
        """        
        self.ui.outButton_1.setStyleSheet("background-color: #f53d3d; color: white;")
        self.ui.outButton_1.clicked.connect(update_callback)
    
    def refresh_port_list(self) -> None:
        """Refresh the list of available ports in the combo box."""
        self.ui.comboBox.clear()
        ports = PortDiscovery.get_available_ports()
        
        for device_name, description in ports:
            display_text = PortDiscovery.format_port_display(device_name, description)
            self.ui.comboBox.addItem(display_text)
    
    def get_selected_port(self) -> Optional[str]:
        """
        Get the currently selected port name.
        
        Returns:
            Port name if valid selection, None otherwise
        """
        port_text = self.ui.comboBox.currentText()
        
        if not port_text.strip() or "n/a" in port_text.lower():
            return None
        
        return PortDiscovery.extract_port_name(port_text)
    
    def set_connection_state(self, connected: bool) -> None:
        """
        Update UI to reflect connection state.
        
        Args:
            connected: True if connected, False if disconnected
        """
        # Update connect button text
        self.ui.connectButton.setText("Disconnect" if connected else "Connect")
        
        # Enable/disable console input
        if self.console_input:
            self.console_input.setEnabled(connected)

    def set_update_buttons_enabled(self, enabled: bool) -> None:
        """
        Enable or disable update buttons in the UI.
        
        Args:
            enabled: True to enable, False to disable
        """
        self.ui.updateFreqButton.setEnabled(enabled)
        self.ui.updateGainButton.setEnabled(enabled)
        self.ui.updateSpreadButton.setEnabled(enabled)
        self.ui.updateBandwidthButton.setEnabled(enabled)

    
    def set_connect_button_enabled(self, enabled: bool) -> None:
        """
        Enable or disable the connect button.
        
        Args:
            enabled: True to enable, False to disable
        """
        self.ui.connectButton.setEnabled(enabled)
    
    def update_rssi_display(self, device_id: str, rssi_value: str) -> None:
        """
        Update RSSI display for a specific device.
        
        Args:
            device_id: Device identifier (SFTU or transceiver)
            rssi_value: RSSI value to display
        """
        if device_id == "SFTU":
            self.ui.thatRSSI.setText("RSSI: " + rssi_value + "dBm")
        elif device_id == "transceiver":
            self.ui.thisRSSI.setText("RSSI: " + rssi_value + "dBm")
    
    def update_device_status_display(self, device_id: str, device_status=None) -> None:
        """
        Update device status display with formatted information.
        
        Args:
            device_id: Device identifier (SFTU or transceiver)
            device_status: DeviceStatus object, or None for default display
        """
        if device_status is None:
            # Default display when no device status is available
            status_text = (
                "ID: None\n"
                "RSSI: -0.00dBm\n"
                "Batt: 0.00V\n"
                "Mode: None\n"
                "Stat: Error"
            )
        else:
            # Format the device status information
            rssi_display = f"{device_status.rssi}dBm" if device_status.rssi != "N/A" else "-0.00dBm"
            batt_display = f"{device_status.battery_voltage}V" if device_status.battery_voltage else "0.00V"
            mode_display = device_status.mode if device_status.mode else "None"
            status_display = device_status.status if device_status.status else "Error"
            
            status_text = (
                f"ID: {device_status.device_id}\n"
                f"RSSI: {rssi_display}\n"
                f"Batt: {batt_display}\n"
                f"Mode: {mode_display}\n"
                f"Stat: {status_display}"
            )
        
        # Update the appropriate display widget
        if device_id == "SFTU":
            widget = self.ui.thatRSSI
        elif device_id == "transceiver":
            widget = self.ui.thisRSSI
        else:
            return
        
        # Set the text
        widget.setText(status_text)
        
        # Ensure proper font size (11pt)
        font = widget.font()
        font.setPointSize(11)
        widget.setFont(font)
    
    def initialize_device_status_displays(self) -> None:
        """Initialize both device status displays with default values."""
        self.update_device_status_display("SFTU", None)
        self.update_device_status_display("transceiver", None)
    
    def initialize_output_button_states(self) -> None:
        """Initialize all output buttons to OFF state (red color)."""
        for i in range(1, 9):  # Buttons 1-8
            button = self.get_output_button(i)
            if button:
                button.setStyleSheet("background-color: #f53d3d; color: white;")  # Red for OFF
    
    def get_console_input_text(self) -> str:
        """
        Get text from console input widget.
        
        Returns:
            Console input text, stripped of whitespace
        """
        if self.console_input:
            return self.console_input.toPlainText().strip()
        return ""
    
    def clear_console_input(self) -> None:
        """Clear the console input widget."""
        if self.console_input:
            self.console_input.clear()
    
    def get_ui(self) -> Ui_main:
        """Get the UI object for direct access if needed."""
        return self.ui
    
    def get_console_input(self) -> Optional[ConsoleInput]:
        """Get the custom console input widget."""
        return self.console_input
    
    def setup_output_buttons(self, update_callback) -> None:
        """
        Set up all 8 output buttons with callbacks.
        
        Args:
            update_callback: Callback function that takes button_index as parameter
        """
        # Connect each output button to the callback with its index
        for i in range(1, 9):  # Buttons 1-8
            button = self.get_output_button(i)
            if button:
                # Use lambda with default parameter to capture the current value of i
                button.clicked.connect(lambda checked, idx=i: update_callback(idx))
    
    def get_output_button(self, button_index: int):
        """
        Get output button by index.
        
        Args:
            button_index: Button index (1-8)
            
        Returns:
            Button widget or None if not found
        """
        button_name = f"outButton_{button_index}"
        return getattr(self.ui, button_name, None)
    
    def get_frequency_value(self) -> str:
        """
        Get the frequency value from the frequency field.
        
        Returns:
            Frequency value as string, stripped of whitespace
        """
        return self.ui.freqField.toPlainText().strip()
    
    def get_bandwidth_value(self) -> str:
        """
        Get the frequency value from the frequency field.
        
        Returns:
            Frequency value as string, stripped of whitespace
        """
        return self.ui.bandwidthField.toPlainText().strip()
    
    def get_spreadingFactor_value(self) -> str:
        """
        Get the spreading factor value from the spreading factor field.
        
        Returns:
            Spreading factor value as string, stripped of whitespace
        """
        return self.ui.spreadingField.toPlainText().strip()
    
    def get_gain_value(self) -> str:
        """
        Get the gain value from the gain field.
        
        Returns:
            Gain value as string, stripped of whitespace
        """
        return self.ui.gainField.toPlainText().strip()

    def validate_value(self, value_str: str, min_val: float, max_val: float) -> tuple:
        """
        Validate the value input.
        
        Args:
            value_str: Value string to validate
            min_val: Minimum valid value
            max_val: Maximum valid value
            
        Returns:
            Tuple of (is_valid, error_message, _value)
        """
        if not value_str:
            return False, "field is empty", 0.0
        
        try:
            _value = float(value_str)
        except ValueError:
            return False, "Invalid frequency format - must be a number", 0.0
        
        if _value < min_val or _value > max_val:
            return False, f"Value must be between {min_val} and {max_val} MHz", 0.0
        
        return True, "", _value
    
    def clear_frequency_field(self) -> None:
        """Clear the frequency input field."""
        self.ui.freqField.clear()
    
    def set_frequency_field_value(self, value: str) -> None:
        """
        Set the frequency field value.
        
        Args:
            value: Frequency value to set
        """
        self.ui.freqField.setPlainText(value)
