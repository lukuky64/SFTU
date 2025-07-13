"""
Alternative implementation using pycparser for more robust C++ parsing.
Install with: pip install pycparser

This is a more advanced option that can handle complex C++ syntax.
"""

try:
    from pycparser import parse_file, c_ast
    PYCPARSER_AVAILABLE = True
except ImportError:
    PYCPARSER_AVAILABLE = False
    print("pycparser not available. Using regex-based parser.")

import os
import re

def parse_with_pycparser(header_path):
    """Parse using pycparser for more robust parsing."""
    if not PYCPARSER_AVAILABLE:
        return None
    
    try:
        # Note: pycparser requires preprocessing for most real C++ files
        # For simple enum definitions, the regex approach might be sufficient
        ast = parse_file(header_path)
        # Implementation would depend on the specific AST structure
        # This is more complex but more reliable for complex C++ code
        pass
    except Exception as e:
        print(f"pycparser failed: {e}")
        return None

def parse_command_id_header_robust(header_path):
    """
    Enhanced parser that can handle more complex C++ syntax.
    """
    commands = {}
    
    try:
        with open(header_path, 'r') as file:
            content = file.read()
        
        # Remove comments first
        content = re.sub(r'//.*?\n', '\n', content)
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        
        # Find enum CommandID block with more flexible regex
        enum_pattern = r'enum\s+CommandID\s*\{([^}]+)\}'
        enum_match = re.search(enum_pattern, content, re.DOTALL | re.IGNORECASE)
        
        if enum_match:
            enum_content = enum_match.group(1)
            
            # Handle both explicit values and auto-increment
            lines = enum_content.split(',')
            current_value = 0
            
            for line in lines:
                line = line.strip()
                if not line:
                    continue
                    
                # Check if explicit value is assigned
                if '=' in line:
                    name_value = line.split('=')
                    name = name_value[0].strip()
                    value = int(name_value[1].strip())
                    commands[name] = value
                    current_value = value + 1
                else:
                    # Auto-increment
                    name = line.strip()
                    if name:
                        commands[name] = current_value
                        current_value += 1
                        
    except Exception as e:
        print(f"Error parsing header file: {e}")
    
    return commands
