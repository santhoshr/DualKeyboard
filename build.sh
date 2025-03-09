#!/bin/bash

APP_NAME="Dual.app"
CONTENTS="$APP_NAME/Contents"

# Clean previous build
rm -rf "$APP_NAME"

# Create bundle structure
mkdir -p "$CONTENTS"/{MacOS,Resources}

# First ensure we have a fresh build
make clean && make

# Copy binary directly from our build output
cp bin/dual "$CONTENTS/MacOS/"
chmod +x "$CONTENTS/MacOS/dual"

# Copy Info.plist
cp Info.plist "$CONTENTS/"

# Sign the app bundle with entitlements
codesign --force --sign - --entitlements entitlements.plist --deep "$APP_NAME"
