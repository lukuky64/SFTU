#!/bin/bash

# Quick build script for SFTU GUI
# This script builds a standalone macOS application

echo "🚀 SFTU GUI Build Script"
echo "========================"

# Check if we're in the right directory
if [ ! -f "main.py" ]; then
    echo "❌ Error: main.py not found. Please run this script from the SFTU-GUI directory."
    exit 1
fi

# Activate virtual environment if it exists
if [ -d "venv" ]; then
    echo "📦 Activating virtual environment..."
    source venv/bin/activate
fi

# Install dependencies
echo "📦 Installing dependencies..."
pip install -r requirements.txt

# Run the Python build script
echo "🔨 Running build process..."
python build_app.py

echo "✅ Build process completed!"
echo "📱 Check the 'dist' folder for your application."
