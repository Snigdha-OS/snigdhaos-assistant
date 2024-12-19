#!/bin/bash

# Function to log messages to a log file
log_message() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" >> /var/log/snigdha_assistant.log
}

# Check if the first file (setup script) exists and execute it
if [ -e "$1" ]; then
    echo ""
    echo "Preparing setup ⏳"
    echo ""
    log_message "Executing setup script: $1"
    if ! sudo bash - <$1; then
        echo "❌ Error executing setup script. Check logs for more details."
        log_message "Error executing setup script: $1"
        exit 1
    fi
else
    echo "⚠️ Setup script ($1) not found! Skipping setup."
    log_message "Setup script ($1) not found."
fi

# Install packages from the second file
echo ""
echo "Installing packages 🛠️"
echo ""
installable_packages=$(comm -12 <(pacman -Slq | sort) <(sed s/\\s/\\n/g - <$2 | sort))

if [ -z "$installable_packages" ]; then
    echo "⚠️ No installable packages found. Skipping package installation."
    log_message "No installable packages found from $2"
else
    echo "📦 The following packages will be installed: $installable_packages"
    log_message "Installing packages: $installable_packages"
    if ! sudo pacman -S --needed $installable_packages; then
        echo "❌ Error installing packages. Check logs for more details."
        log_message "Error installing packages: $installable_packages"
        exit 1
    fi
    rm -f $2
    echo "✅ Packages installed successfully."
    log_message "Packages installed successfully."
fi

# Check if the third file (service script) exists and execute it
if [ -e "$3" ]; then
    echo ""
    echo "Enabling services ⚙️"
    echo ""
    log_message "Enabling services from: $3"
    if ! sudo bash - <$3; then
        echo "❌ Error enabling services. Check logs for more details."
        log_message "Error enabling services from: $3"
        exit 1
    fi
else
    echo "⚠️ Service script ($3) not found! Skipping services."
    log_message "Service script ($3) not found."
fi

# Final prompt
echo ""
read -p "Press enter to return to Snigdha OS Assistant 🛑"

# End of script
log_message "Script execution completed successfully 🎉"
