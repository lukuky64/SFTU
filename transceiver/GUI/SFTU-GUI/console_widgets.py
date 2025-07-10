"""
Console widgets for the SFTU GUI application.
Handles custom input widget and console output management.
"""

import html
from typing import List
from PySide6.QtCore import Qt, Signal, QTimer
from PySide6.QtWidgets import QTextEdit


class ConsoleInput(QTextEdit):
    """Custom text input widget that emits a signal when Enter is pressed."""
    
    returnPressed = Signal()

    def keyPressEvent(self, event):
        """Handle key press events, specifically Enter key."""
        if event.key() in (Qt.Key_Return, Qt.Key_Enter):
            self.returnPressed.emit()
            event.accept()  # Prevent newline insertion
        else:
            super().keyPressEvent(event)


class ConsoleManager:
    """Manages console output with buffering and formatting."""
    
    # Color scheme for different message types
    MESSAGE_COLORS = {
        "user": "#FFB6C1",      # Light pink for user commands
        "incoming": "#B0E0E6",   # Light blue for incoming data
        "system": "#FFD580",     # Light orange for system messages
        "normal": "#ffffff"      # White for normal text
    }
    
    def __init__(self, console_output_widget: QTextEdit):
        """
        Initialize console manager.
        
        Args:
            console_output_widget: The QTextEdit widget for console output
        """
        self.console_output = console_output_widget
        self.console_buffer: List[str] = []
        self.max_console_blocks = 500  # Prevent memory issues
        
        # Set up buffer timer for efficient output
        self.buffer_timer = QTimer()
        self.buffer_timer.setInterval(100)  # 100ms buffer delay
        self.buffer_timer.timeout.connect(self.flush_buffer)
        self.buffer_timer.start()
    
    def append_message(self, text: str, msg_type: str = "normal") -> None:
        """
        Add a message to the console buffer.
        
        Args:
            text: The message text to display
            msg_type: Type of message (user, incoming, system, normal)
        """
        color = self.MESSAGE_COLORS.get(msg_type, self.MESSAGE_COLORS["normal"])
        safe_text = html.escape(text)
        # Use a font family that is more likely to exist (Menlo, Monaco, monospace)
        html_line = (
            f'<span style="color:{color}; font-family:Menlo, Monaco, monospace;">'
            f'{safe_text}</span><br>'
        )
        self.console_buffer.append(html_line)
    
    def flush_buffer(self) -> None:
        """Flush the console buffer to the output widget."""
        if self.console_buffer:
            self.console_output.insertHtml(''.join(self.console_buffer))
            self.console_buffer.clear()
            self._scroll_to_bottom()
            self._limit_console_size()
    
    def clear(self) -> None:
        """Clear the console output."""
        self.console_output.clear()
    
    def _scroll_to_bottom(self) -> None:
        """Scroll console to the bottom."""
        scrollbar = self.console_output.verticalScrollBar()
        scrollbar.setValue(scrollbar.maximum())
    
    def _limit_console_size(self) -> None:
        """Limit console size to prevent performance issues."""
        doc = self.console_output.document()
        if doc.blockCount() > self.max_console_blocks:
            cursor = self.console_output.textCursor()
            cursor.movePosition(cursor.Start)
            # Remove excess blocks from the beginning
            blocks_to_remove = doc.blockCount() - self.max_console_blocks
            for _ in range(blocks_to_remove):
                cursor.select(cursor.LineUnderCursor)
                cursor.removeSelectedText()
                cursor.deleteChar()
