#!/bin/bash

# Create virtual environment
python3 -m venv venv

# Activate virtual environment
source venv/bin/activate

# Install matplotlib
pip3 install matplotlib

echo "Setup complete. Virtual environment created and matplotlib installed."