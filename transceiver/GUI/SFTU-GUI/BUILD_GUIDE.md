# SFTU GUI - Building Standalone macOS Application

This guide explains how to create a standalone macOS application (.app bundle) from your SFTU GUI Python project.

## Prerequisites

- macOS system
- Python 3.8 or later
- All project dependencies installed

## Quick Build (Recommended)

### Option 1: Using the automated build script

1. Open Terminal and navigate to the SFTU-GUI directory:
   ```bash
   cd /Users/lucas/Documents/GitHub/SFTU/transceiver/GUI/SFTU-GUI
   ```

2. Run the build script:
   ```bash
   ./build.sh
   ```

### Option 2: Using the Python build script

1. Navigate to the project directory:
   ```bash
   cd /Users/lucas/Documents/GitHub/SFTU/transceiver/GUI/SFTU-GUI
   ```

2. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```

3. Run the build script:
   ```bash
   python build_app.py
   ```

## Manual Build Process

If you prefer to build manually or customize the process:

1. Install PyInstaller:
   ```bash
   pip install pyinstaller
   ```

2. Build using the spec file:
   ```bash
   pyinstaller SFTU-GUI.spec
   ```

3. Or build with command line options:
   ```bash
   pyinstaller --name=SFTU-GUI --windowed --onedir --add-data=form.ui:. --clean --noconfirm main.py
   ```

## Output

After successful building, you'll find:

- `dist/SFTU-GUI.app` - The standalone macOS application
- `dist/SFTU-GUI/` - Directory containing all dependencies
- `build/` - Temporary build files (can be deleted)

## Distribution

### For Personal Use:
1. Move `SFTU-GUI.app` to your Applications folder
2. Double-click to run

### For Distribution to Others:
1. Compress the entire `dist` folder into a ZIP file
2. Share the ZIP file with other users
3. Recipients should extract and move the app to their Applications folder

## Troubleshooting

### Common Issues:

1. **"SFTU-GUI.app" is damaged and can't be opened**
   - Run: `xattr -cr dist/SFTU-GUI.app`
   - This removes quarantine attributes

2. **Permission denied**
   - Run: `chmod +x dist/SFTU-GUI.app/Contents/MacOS/SFTU-GUI`

3. **Missing dependencies**
   - Check that all Python packages are installed
   - Verify the requirements.txt file is complete

4. **Large file size**
   - The app bundle will be larger (~100-200MB) because it includes Python runtime
   - This is normal for standalone Python applications

### Advanced Options:

- **Code Signing**: For distribution outside the Mac App Store
- **Notarization**: Required for distribution to other users on macOS 10.15+
- **Custom Icon**: Add an `.icns` file and update the build script

## File Structure

```
dist/
├── SFTU-GUI.app/          # The macOS application bundle
│   ├── Contents/
│   │   ├── Info.plist     # App metadata
│   │   ├── MacOS/         # Executable files
│   │   └── Resources/     # App resources
└── SFTU-GUI/              # Dependencies directory
    ├── main               # Main executable
    ├── form.ui            # UI file
    └── [various libs]     # Python libraries and dependencies
```

## Notes

- The first run may take longer as macOS verifies the application
- Serial port access should work without additional permissions
- The application is self-contained and doesn't require Python to be installed on the target machine
