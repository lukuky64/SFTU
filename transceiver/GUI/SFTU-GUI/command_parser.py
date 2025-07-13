"""
Utility to parse commandID.hpp and extract CommandID enum values.
"""
import re
import os

def parse_command_id_header(header_path):
    """
    Parse the commandID.hpp file and extract enum values.
    
    Args:
        header_path: Path to the commandID.hpp file
        
    Returns:
        dict: Dictionary mapping command names to their values
    """
    commands = {}
    
    try:
        with open(header_path, 'r') as file:
            content = file.read()
            
        # Find the enum block
        enum_pattern = r'enum\s+CommandID\s*\{([^}]+)\}'
        enum_match = re.search(enum_pattern, content, re.DOTALL)
        
        if enum_match:
            enum_content = enum_match.group(1)
            
            # Extract individual enum values
            # Pattern matches: CMD_NAME = value,
            value_pattern = r'(\w+)\s*=\s*(\d+)'
            matches = re.findall(value_pattern, enum_content)
            
            for name, value in matches:
                commands[name] = int(value)
                
    except FileNotFoundError:
        print(f"Warning: Could not find {header_path}")
    except Exception as e:
        print(f"Error parsing header file: {e}")
    
    return commands

def get_command_ids():
    """
    Get CommandID enum values from the header file.
    
    Returns:
        dict: Dictionary of command IDs
    """
    # Path to the commandID.hpp file relative to this script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    header_path = os.path.join(script_dir, '..', '..', '..', 'shared_lib', 'commander', 'commandID.hpp')
    
    return parse_command_id_header(header_path)

# Create a class dynamically from the parsed header
def create_command_id_class():
    """
    Create a CommandID class with values from the header file.
    
    Returns:
        type: CommandID class with enum values as class attributes
    """
    command_dict = get_command_ids()
    
    # Create a new class with the parsed values
    return type('CommandID', (), command_dict)

# For backwards compatibility, create the CommandID class
CommandID = create_command_id_class()
