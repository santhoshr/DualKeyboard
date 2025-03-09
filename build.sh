#!/bin/bash

APP_NAME="Dual.app"
CONTENTS="$APP_NAME/Contents"

# Clean previous build
rm -rf "$APP_NAME"

# Create bundle structure
mkdir -p "$CONTENTS"/{MacOS,Resources}

# Copy binary
cp /usr/local/bin/dual "$CONTENTS/MacOS/"
chmod +x "$CONTENTS/MacOS/dual"

# Copy Info.plist
cp Info.plist "$CONTENTS/"

# Sign the app bundle with entitlements
codesign --force --sign - --entitlements entitlements.plist --deep "$APP_NAME"
