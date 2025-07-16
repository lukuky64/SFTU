#!/usr/bin/env python3
"""
Build script for creating a standalone SFTU GUI application.
This script uses PyInstaller to create a macOS .app bundle.
"""

import os
import sys
import subprocess
from pathlib import Path

def run_command(cmd, description):
    """Run a command and handle errors."""
    print(f"\n{description}...")
    print(f"Running: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(f"✅ {description} completed successfully")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ {description} failed:")
        print(f"Exit code: {e.returncode}")
        print(f"STDERR: {e.stderr}")
        print(f"STDOUT: {e.stdout}")
        return False

def main():
    """Main build process."""
    print("🚀 Building SFTU GUI standalone application...")
    
    # Check if we're in the right directory
    if not Path("main.py").exists():
        print("❌ Error: main.py not found. Please run this script from the SFTU-GUI directory.")
        sys.exit(1)
    
    # Install PyInstaller if not already installed
    print("\n📦 Installing PyInstaller...")
    if not run_command([sys.executable, "-m", "pip", "install", "pyinstaller"], "Installing PyInstaller"):
        sys.exit(1)
    
    # Clean previous builds
    print("\n🧹 Cleaning previous builds...")
    for path in ["build", "dist", "__pycache__"]:
        if Path(path).exists():
            run_command(["rm", "-rf", path], f"Removing {path}")
    
    # Build the application
    print("\n🔨 Building application...")
    pyinstaller_cmd = [
        sys.executable, "-m", "PyInstaller",
        "--name=SFTU-GUI",
        "--windowed",  # No console window
        "--onedir",    # Create a directory with all dependencies
        "--icon=icon.icns",  # Will be created if doesn't exist
        "--add-data=form.ui:.",  # Include the UI file
        "--clean",
        "--noconfirm",
        "main.py"
    ]
    
    if not run_command(pyinstaller_cmd, "Building application with PyInstaller"):
        print("\n❌ Build failed. Trying without icon...")
        # Try again without icon
        pyinstaller_cmd_no_icon = [
            sys.executable, "-m", "PyInstaller",
            "--name=SFTU-GUI",
            "--windowed",
            "--onedir",
            "--add-data=form.ui:.",
            "--clean",
            "--noconfirm",
            "main.py"
        ]
        
        if not run_command(pyinstaller_cmd_no_icon, "Building application without icon"):
            sys.exit(1)
    
    # Check if build was successful
    app_path = Path("dist/SFTU-GUI.app")
    if app_path.exists():
        print(f"\n✅ Build successful!")
        print(f"📱 Application created at: {app_path.absolute()}")
        print(f"💾 Size: {get_folder_size(app_path):.1f} MB")
        print(f"\n🎯 You can now:")
        print(f"   1. Open the app by double-clicking: {app_path}")
        print(f"   2. Move it to Applications folder")
        print(f"   3. Distribute the entire 'dist' folder to other users")
    else:
        print("\n❌ Build completed but application not found!")
        sys.exit(1)

def get_folder_size(folder_path):
    """Get folder size in MB."""
    total_size = 0
    for dirpath, dirnames, filenames in os.walk(folder_path):
        for filename in filenames:
            filepath = os.path.join(dirpath, filename)
            if os.path.exists(filepath):
                total_size += os.path.getsize(filepath)
    return total_size / (1024 * 1024)  # Convert to MB

if __name__ == "__main__":
    main()
