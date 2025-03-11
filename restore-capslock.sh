#!/bin/bash

# restore-capslock.sh
# This script restores the original CapsLock functionality if the Dual program
# aborts unexpectedly without properly cleaning up.

echo "Restoring original CapsLock functionality..."

# Clear the UserKeyMapping to restore default behavior
hidutil property --set '{"UserKeyMapping":[]}'

# Check if the command was successful
if [ $? -eq 0 ]; then
    echo "✅ CapsLock has been restored to its original functionality."
    echo "You can now use CapsLock normally."
else
    echo "❌ Failed to restore CapsLock functionality."
    echo "You may need to run this script with sudo:"
    echo "sudo ./restore-capslock.sh"
    exit 1
fi

# Check if the lock file exists and remove it
if [ -f "/tmp/Dual.lock" ]; then
    echo "Removing Dual lock file..."
    rm -f "/tmp/Dual.lock"
    if [ $? -eq 0 ]; then
        echo "✅ Lock file removed successfully."
    else
        echo "❌ Failed to remove lock file. You may need to run with sudo."
    fi
fi

echo ""
echo "You can now restart the Dual program if desired."
echo "" 