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
