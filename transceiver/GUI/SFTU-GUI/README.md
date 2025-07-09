# SFTU GUI Application

A PySide6-based GUI application for communicating with SFTU (Short Form Transceiver Unit) devices.

## Project Structure

```
SFTU-GUI/
├── main.py              # Main application entry point
├── ui_form.py           # Generated UI form (from Qt Designer)
├── form.ui              # Qt Designer UI file
├── console_widgets.py   # Console input widget and output manager
├── serial_handler.py    # Serial communication management
├── status_parser.py     # Device status message parsing
├── ui_manager.py        # UI setup and management
├── requirements.txt     # Python dependencies
└── pyproject.toml       # Project configuration
```

## Architecture Overview

The application follows object-oriented programming principles with clear separation of concerns:

### Core Classes

1. **SFTUApplication** (`main.py`)
   - Main application window
   - Orchestrates all components
   - Handles high-level application logic

2. **UIManager** (`ui_manager.py`)
   - Manages UI setup and configuration
   - Handles widget replacements and styling
   - Provides UI state management methods

3. **ConsoleManager** (`console_widgets.py`)
   - Manages console output with buffering
   - Handles message formatting and colors
   - Provides console size limiting

4. **ConsoleInput** (`console_widgets.py`)
   - Custom text input widget
   - Handles Enter key press events
   - Emits signals for command submission

5. **SerialPort** (`serial_handler.py`)
   - Handles serial communication
   - Manages connection state
   - Provides data transmission and reception

6. **PortDiscovery** (`serial_handler.py`)
   - Utility for discovering available ports
   - Handles port formatting and parsing

7. **StatusParser** (`status_parser.py`)
   - Parses incoming device status messages
   - Extracts device information (RSSI, battery, etc.)

8. **StatusManager** (`status_parser.py`)
   - Manages device status information
   - Handles RSSI display updates
   - Stores device state

## Key Features

- **Modular Design**: Each component has a single responsibility
- **Signal-Based Communication**: Uses Qt signals for loose coupling
- **Error Handling**: Comprehensive error handling throughout
- **Buffered Console Output**: Efficient console display with size limits
- **Automatic Port Discovery**: Dynamic serial port detection
- **Device Status Parsing**: Automatic parsing of device status messages
- **Configurable Styling**: Color-coded console messages

## Usage

Run the application:
```bash
python main.py
```

## Dependencies

- PySide6: Qt-based GUI framework
- pyserial: Serial communication library

## Development Notes

- The UI form (`ui_form.py`) is generated from Qt Designer and should not be edited manually
- Custom widgets are dynamically replaced at runtime
- All business logic is separated from UI concerns
- The application uses a non-blocking approach for serial communication
