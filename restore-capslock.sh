#!/bin/bash

# restore-capslock.sh
# This script restores the original CapsLock functionality if the dual program
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
if [ -f "/tmp/dual.lock" ]; then
    echo "Removing dual lock file..."
    rm -f "/tmp/dual.lock"
    if [ $? -eq 0 ]; then
        echo "✅ Lock file removed successfully."
    else
        echo "❌ Failed to remove lock file. You may need to run with sudo."
    fi
fi

echo ""
echo "You can now restart the dual program if desired."
echo "" 