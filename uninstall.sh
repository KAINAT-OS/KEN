#!/usr/bin/env bash
# ============================================================================
# uninstall.sh – Remove Ken language toolchain from the system
#  Requires: sudo / root (to remove files from /usr/local)
#  Usage:    sudo ./uninstall.sh
# ============================================================================

set -euo pipefail

INSTALL_PREFIX="/usr/local"
BIN_DIR="$INSTALL_PREFIX/bin"
INCLUDE_DIR="$INSTALL_PREFIX/include"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}🌸 Ken Language Toolchain Uninstaller${NC}"
echo "=============================================="

# --- privilege check -------------------------------------------------------
if [[ $EUID -ne 0 ]]; then
    echo -e "${RED}This script must be run as root (use sudo).${NC}"
    exit 1
fi

# --- files to remove -------------------------------------------------------
FILES_TO_REMOVE=(
    "$BIN_DIR/ken_prep"
    "$BIN_DIR/kencc"
    "$INCLUDE_DIR/ken.hpp"
)

# --- show what will be removed ---------------------------------------------
echo -e "${YELLOW}The following files will be removed:${NC}"
for file in "${FILES_TO_REMOVE[@]}"; do
    if [[ -f "$file" ]]; then
        echo "  $file"
    else
        echo "  $file (not found)"
    fi
done
echo ""

read -p "Are you sure you want to proceed? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Uninstall cancelled."
    exit 0
fi

# --- remove files ----------------------------------------------------------
echo "Removing files..."
for file in "${FILES_TO_REMOVE[@]}"; do
    if [[ -f "$file" ]]; then
        rm -f "$file"
        echo -e "${GREEN}✔ Removed: $file${NC}"
    else
        echo -e "${YELLOW}⚠ Skipped: $file (does not exist)${NC}"
    fi
done

# --- optionally remove empty directories -----------------------------------
if [[ -d "$BIN_DIR" ]] && [[ -z "$(ls -A "$BIN_DIR" 2>/dev/null)" ]]; then
    echo -e "${YELLOW}Note: $BIN_DIR is now empty. You may remove it manually if desired.${NC}"
fi

if [[ -d "$INCLUDE_DIR" ]] && [[ -z "$(ls -A "$INCLUDE_DIR" 2>/dev/null)" ]]; then
    echo -e "${YELLOW}Note: $INCLUDE_DIR is now empty. You may remove it manually if desired.${NC}"
fi

# --- final message ---------------------------------------------------------
echo ""
echo "=============================================="
echo -e "${GREEN}✅ Ken language toolchain uninstalled successfully.${NC}"
echo ""
echo "Removed:"
for file in "${FILES_TO_REMOVE[@]}"; do
    echo "  • $file"
done
echo ""
echo "If you added the $BIN_DIR to your PATH manually, please remove it from your shell profile."