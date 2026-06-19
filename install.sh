#!/usr/bin/env bash
# ============================================================================
# Ken By Kainat Quaderee. easy c++ without compiller modify.
#  install.sh – Install the Ken language toolchain system‑wide
#  Requires: sudo / root, g++ (with C++23 support)
#  Usage:    sudo ./install.sh
# ============================================================================

set -euo pipefail

INSTALL_PREFIX="/usr/local"
BIN_DIR="$INSTALL_PREFIX/bin"
INCLUDE_DIR="$INSTALL_PREFIX/include"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}🌸 Ken Language Toolchain Installer${NC}"
echo "=============================================="

# --- privilege check -------------------------------------------------------
if [[ $EUID -ne 0 ]]; then
    echo -e "${RED}This script must be run as root (use sudo).${NC}"
    exit 1
fi

# --- check for g++ ---------------------------------------------------------
if ! command -v g++ &> /dev/null; then
    echo -e "${RED}g++ not found. Please install GCC with C++23 support.${NC}"
    exit 1
fi

# --- check C++23 support (simple compile test) -----------------------------
echo "Checking C++23 support..."
echo 'int main() { return 0; }' > /tmp/ken_check_cpp23.cpp
if ! g++ -std=c++23 /tmp/ken_check_cpp23.cpp -o /tmp/ken_check_cpp23 &>/dev/null; then
    rm -f /tmp/ken_check_cpp23.cpp /tmp/ken_check_cpp23
    echo -e "${RED}Your g++ does not support -std=c++23. Please upgrade.${NC}"
    exit 1
fi
rm -f /tmp/ken_check_cpp23.cpp /tmp/ken_check_cpp23
echo -e "${GREEN}Compiler is ready.${NC}"

# --- destination directories -----------------------------------------------
mkdir -p "$BIN_DIR" "$INCLUDE_DIR"

# --- 1. Install ken.hpp -----------------------------------------------------
echo "Installing ken.hpp to $INCLUDE_DIR ..."
if [[ -f "$INCLUDE_DIR/ken.hpp" ]]; then
    echo -e "${YELLOW}Existing ken.hpp found. Overwriting...${NC}"
fi
cp -f ken.hpp "$INCLUDE_DIR/ken.hpp"
chmod 644 "$INCLUDE_DIR/ken.hpp"
echo -e "${GREEN}✔ ken.hpp installed.${NC}"

# --- 2. Compile ken_prep ----------------------------------------------------
echo "Compiling ken_prep (semicolon preprocessor)..."
if [[ -f "$BIN_DIR/ken_prep" ]]; then
    echo -e "${YELLOW}Existing ken_prep found. Overwriting...${NC}"
fi

# Compile with optimisation for maximum speed
g++ -std=c++23 -O3 -Wall -Wextra ken_prep.cpp -o "$BIN_DIR/ken_prep"
chmod 755 "$BIN_DIR/ken_prep"
echo -e "${GREEN}✔ ken_prep compiled and installed.${NC}"

# --- 3. Install kencc wrapper script ---------------------------------------
echo "Installing kencc compiler driver..."
if [[ -f "$BIN_DIR/kencc" ]]; then
    echo -e "${YELLOW}Existing kencc found. Overwriting...${NC}"
fi
cp -f kencc "$BIN_DIR/kencc"
chmod 755 "$BIN_DIR/kencc"
echo -e "${GREEN}✔ kencc installed.${NC}"

# --- final message ---------------------------------------------------------
echo ""
echo "=============================================="
echo -e "${GREEN}✅ Ken language toolchain installed successfully!${NC}"
echo ""
echo "  • ken.hpp       → $INCLUDE_DIR/ken.hpp"
echo "  • ken_prep      → $BIN_DIR/ken_prep"
echo "  • kencc         → $BIN_DIR/kencc"
echo ""
echo "You can now compile Ken programs with:"
echo "  kencc myprogram.ken -o myprogram"
echo ""
