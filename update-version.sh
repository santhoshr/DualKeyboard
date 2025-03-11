#!/bin/bash

# This script updates the version number in all relevant files 
# Usage: ./update-version.sh <new_version>

if [ $# -ne 1 ]; then
    echo "Usage: $0 <new_version>"
    echo "Example: $0 4.1.0"
    exit 1
fi

NEW_VERSION="$1"
OLD_VERSION=$(cat VERSION)

echo "Updating version from $OLD_VERSION to $NEW_VERSION"

# Update the VERSION file
echo -n "$NEW_VERSION" > VERSION

# Update version in README.md
sed -i '' -E "s/(Toggle \*\*Key Display\*\*  \(v)[0-9]+(\.[0-9]+)*(\.[0-9]+)*/\1$NEW_VERSION/g" README.md
sed -i '' -E "s/(Dual v)[0-9]+(\.[0-9]+)*(\.[0-9]+)*/\1$NEW_VERSION/g" README.md
sed -i '' -E "s/^# v[0-9]+(\.[0-9]+)*(\.[0-9]+)*/# v$NEW_VERSION/g" README.md

# Add new changelog entry in README.md if needed
if ! grep -q "### \[v$NEW_VERSION\]" README.md; then
    DATE=$(date +%Y-%m-%d)
    # Find the line number where the changelog section starts
    CHANGELOG_START=$(grep -n "^## Changelog" README.md | cut -d: -f1)
    if [ ! -z "$CHANGELOG_START" ]; then
        # Add 1 to get to the line after "## Changelog"
        INSERT_LINE=$((CHANGELOG_START + 1))
        sed -i '' "${INSERT_LINE}i\\
\\
### [v$NEW_VERSION]\\
- Version update\\
" README.md
    fi
fi

echo "Version updated successfully. Please check the following files:"
echo "- VERSION"
echo "- README.md"
echo ""
echo "Remember to build the app to apply the changes: ./build.sh"