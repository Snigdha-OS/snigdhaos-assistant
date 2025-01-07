#!/bin/bash

# Function to log messages to a log file
log_message() {
    local log_file="/var/log/snigdhaos_assistant.log"
    if [ -w "$log_file" ] || sudo touch "$log_file" 2>/dev/null; then
        echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" | sudo tee -a "$log_file" > /dev/null
    else
        echo "Log file not writable: $log_file"
    fi
}

# Check if the first file (setup script) exists and execute it
if [ -e "$1" ]; then
    echo ""
    echo "Preparing setup ⏳"  # Show waiting emoji when preparing setup
    echo ""
    log_message "Executing setup script: $1"
    if ! sudo bash - <$1; then
        # If the setup script execution fails, print an error message with a cross emoji
        echo "❌ Error executing setup script. Check logs for more details."
        log_message "Error executing setup script: $1"
        exit 1  # Exit the script if setup fails
    fi
else
    # If the setup script doesn't exist, show a warning with a caution emoji
    echo "⚠️ Setup script ($1) not found! Skipping setup."
    log_message "Setup script ($1) not found."
fi

# Install packages from the second file
echo ""
echo "Installing packages 🛠️"  # Show tools emoji for package installation
echo ""
installable_packages=$(comm -12 <(pacman -Slq | sort) <(sed s/\\s/\\n/g - <$2 | sort))  # Find common packages between the pacman repo and the list in $2

if [ -z "$installable_packages" ]; then
    # If no installable packages are found, show a warning with a caution emoji
    echo "⚠️ No installable packages found. Skipping package installation."
    log_message "No installable packages found from $2"
else
    # If packages are found, list them and show a package emoji
    echo "📦 The following packages will be installed: $installable_packages"
    log_message "Installing packages: $installable_packages"
    if ! sudo pacman -S --needed $installable_packages --noconfirm; then
        # If the package installation fails, show an error with a cross emoji
        echo "❌ Error installing packages. Check logs for more details."
        log_message "Error installing packages: $installable_packages"
        exit 1  # Exit the script if package installation fails
    fi
    rm -f $2  # Remove the file after successful installation
    echo "✅ Packages installed successfully."  # Show success emoji after installation
    log_message "Packages installed successfully."
fi

# Check if the third file (service script) exists and execute it
if [ -e "$3" ]; then
    echo ""
    echo "Enabling services ⚙️"  # Show gear emoji for service enabling
    echo ""
    log_message "Enabling services from: $3"
    if ! sudo bash - <$3; then
        # If enabling services fails, show an error with a cross emoji
        echo "❌ Error enabling services. Check logs for more details."
        log_message "Error enabling services from: $3"
        exit 1  # Exit the script if enabling services fails
    fi
else
    # If the service script doesn't exist, show a warning with a caution emoji
    echo "⚠️ Service script ($3) not found! Skipping services."
    log_message "Service script ($3) not found."
fi

# Final prompt
echo ""
read -p "Press enter to return to Snigdha OS Assistant 🛑"  # Show stop sign emoji for returning to the assistant

# End of script
log_message "Script execution completed successfully 🎉"  # Log success with a celebration emoji
