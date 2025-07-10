"""
Unit tests for the SFTU GUI application components.
Demonstrates the modular, testable design of the refactored code.
"""

import unittest
from unittest.mock import Mock, patch
import sys
import os

# Add the project directory to the path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from status_parser import StatusParser, StatusManager, DeviceStatus
from serial_handler import PortDiscovery


class TestStatusParser(unittest.TestCase):
    """Test cases for the StatusParser class."""
    
    def setUp(self):
        """Set up test fixtures."""
        self.parser = StatusParser()
    
    def test_parse_valid_status_line(self):
        """Test parsing a valid status line."""
        line = "status ID:SFTU RSSI:-45 battVoltage:3.7 mode:active status:ok"
        result = self.parser.parse_status_line(line)
        
        self.assertIsNotNone(result)
        self.assertEqual(result.device_id, "SFTU")
        self.assertEqual(result.rssi, "-45")
        self.assertEqual(result.battery_voltage, "3.7")
        self.assertEqual(result.mode, "active")
        self.assertEqual(result.status, "ok")
    
    def test_parse_invalid_status_line(self):
        """Test parsing an invalid status line."""
        line = "some random message"
        result = self.parser.parse_status_line(line)
        
        self.assertIsNone(result)
    
    def test_parse_status_line_missing_id(self):
        """Test parsing a status line without ID."""
        line = "status RSSI:-45 battVoltage:3.7"
        result = self.parser.parse_status_line(line)
        
        self.assertIsNone(result)
    
    def test_parse_transceiver_status(self):
        """Test parsing transceiver status."""
        line = "status ID:transceiver RSSI:-30 mode:transmit"
        result = self.parser.parse_status_line(line)
        
        self.assertIsNotNone(result)
        self.assertEqual(result.device_id, "transceiver")
        self.assertEqual(result.rssi, "-30")
        self.assertEqual(result.mode, "transmit")
    
    def test_is_known_device(self):
        """Test device recognition."""
        self.assertTrue(self.parser.is_known_device("SFTU"))
        self.assertTrue(self.parser.is_known_device("transceiver"))
        self.assertFalse(self.parser.is_known_device("unknown_device"))


class TestStatusManager(unittest.TestCase):
    """Test cases for the StatusManager class."""
    
    def setUp(self):
        """Set up test fixtures."""
        self.status_callback = Mock()
        self.manager = StatusManager(self.status_callback)
    
    def test_process_valid_status_line(self):
        """Test processing a valid status line."""
        line = "status ID:SFTU RSSI:-45 battVoltage:3.7"
        result = self.manager.process_status_line(line)
        
        self.assertIsNotNone(result)
        self.assertEqual(result.device_id, "SFTU")
        
        # Check that status callback was called with device_id and DeviceStatus object
        self.status_callback.assert_called_once()
        args = self.status_callback.call_args[0]
        self.assertEqual(args[0], "SFTU")  # device_id
        self.assertIsInstance(args[1], DeviceStatus)  # DeviceStatus object
        self.assertEqual(args[1].device_id, "SFTU")
        self.assertEqual(args[1].rssi, "-45")
    
    def test_process_invalid_status_line(self):
        """Test processing an invalid status line."""
        line = "invalid message"
        result = self.manager.process_status_line(line)
        
        self.assertIsNone(result)
        self.status_callback.assert_not_called()
    
    def test_get_device_status(self):
        """Test retrieving device status."""
        line = "status ID:SFTU RSSI:-45 battVoltage:3.7"
        self.manager.process_status_line(line)
        
        status = self.manager.get_device_status("SFTU")
        self.assertIsNotNone(status)
        self.assertEqual(status.device_id, "SFTU")
        self.assertEqual(status.rssi, "-45")
        
        # Test non-existent device
        status = self.manager.get_device_status("nonexistent")
        self.assertIsNone(status)
    
    def test_clear_statuses(self):
        """Test clearing all device statuses."""
        line = "status ID:SFTU RSSI:-45 battVoltage:3.7"
        self.manager.process_status_line(line)
        
        # Verify status exists
        self.assertIsNotNone(self.manager.get_device_status("SFTU"))
        
        # Clear and verify
        self.manager.clear_statuses()
        self.assertIsNone(self.manager.get_device_status("SFTU"))


class TestPortDiscovery(unittest.TestCase):
    """Test cases for the PortDiscovery class."""
    
    def test_format_port_display(self):
        """Test port display formatting."""
        result = PortDiscovery.format_port_display("COM3", "USB Serial Port")
        expected = "COM3 — USB Serial Port"
        self.assertEqual(result, expected)
    
    def test_extract_port_name(self):
        """Test port name extraction."""
        formatted = "COM3 — USB Serial Port"
        result = PortDiscovery.extract_port_name(formatted)
        self.assertEqual(result, "COM3")
    
    @patch('serial.tools.list_ports.comports')
    def test_get_available_ports(self, mock_comports):
        """Test getting available ports."""
        # Mock port objects
        mock_port1 = Mock()
        mock_port1.device = "COM3"
        mock_port1.description = "USB Serial Port"
        
        mock_port2 = Mock()
        mock_port2.device = "COM4"
        mock_port2.description = "n/a"  # Should be filtered out
        
        mock_port3 = Mock()
        mock_port3.device = "/dev/ttyUSB0"
        mock_port3.description = "FTDI USB Serial Device"
        
        mock_comports.return_value = [mock_port1, mock_port2, mock_port3]
        
        result = PortDiscovery.get_available_ports()
        
        # Should exclude the port with "n/a" description
        expected = [
            ("COM3", "USB Serial Port"),
            ("/dev/ttyUSB0", "FTDI USB Serial Device")
        ]
        self.assertEqual(result, expected)


class TestDeviceStatus(unittest.TestCase):
    """Test cases for the DeviceStatus dataclass."""
    
    def test_device_status_creation(self):
        """Test creating a DeviceStatus object."""
        status = DeviceStatus(
            device_id="SFTU",
            rssi="-45",
            battery_voltage="3.7",
            mode="active",
            status="ok"
        )
        
        self.assertEqual(status.device_id, "SFTU")
        self.assertEqual(status.rssi, "-45")
        self.assertEqual(status.battery_voltage, "3.7")
        self.assertEqual(status.mode, "active")
        self.assertEqual(status.status, "ok")
    
    def test_device_status_defaults(self):
        """Test DeviceStatus with default values."""
        status = DeviceStatus(device_id="test")
        
        self.assertEqual(status.device_id, "test")
        self.assertEqual(status.rssi, "N/A")
        self.assertEqual(status.battery_voltage, "")
        self.assertEqual(status.mode, "")
        self.assertEqual(status.status, "")


class TestUIManager(unittest.TestCase):
    """Test cases for the UIManager frequency validation."""
    
    def test_validate_frequency_valid(self):
        """Test frequency validation with valid values."""
        from ui_manager import UIManager
        from unittest.mock import Mock
        
        # Mock the main window and UI
        mock_window = Mock()
        
        # We'll test the validation logic directly
        ui_manager = UIManager.__new__(UIManager)  # Create without calling __init__
        
        # Test valid frequencies (within the 900-1000 MHz range)
        result = ui_manager.validate_frequency("950.5")
        valid, msg, value = result
        self.assertTrue(valid)
        self.assertEqual(msg, "")
        self.assertEqual(value, 950.5)
        
        valid, msg, value = ui_manager.validate_frequency("1000")
        self.assertTrue(valid)
        self.assertEqual(msg, "")
        self.assertEqual(value, 1000.0)
        
        valid, msg, value = ui_manager.validate_frequency("900")
        self.assertTrue(valid)
        self.assertEqual(msg, "")
        self.assertEqual(value, 900.0)
    
    def test_validate_frequency_invalid_format(self):
        """Test frequency validation with invalid format."""
        from ui_manager import UIManager
        
        ui_manager = UIManager.__new__(UIManager)
        
        # Test invalid formats
        valid, msg, value = ui_manager.validate_frequency("abc")
        self.assertFalse(valid)
        self.assertIn("Invalid frequency format", msg)
        
        valid, msg, value = ui_manager.validate_frequency("")
        self.assertFalse(valid)
        self.assertIn("Frequency field is empty", msg)
    
    def test_validate_frequency_out_of_range(self):
        """Test frequency validation with out of range values."""
        from ui_manager import UIManager
        
        ui_manager = UIManager.__new__(UIManager)
        
        # Test out of range values (based on 900-1000 MHz range)
        valid, msg, value = ui_manager.validate_frequency("50")  # Too low
        self.assertFalse(valid)
        self.assertIn("Frequency must be between", msg)
        
        valid, msg, value = ui_manager.validate_frequency("1500")  # Too high
        self.assertFalse(valid)
        self.assertIn("Frequency must be between", msg)


if __name__ == "__main__":
    # Run the tests
    unittest.main()
