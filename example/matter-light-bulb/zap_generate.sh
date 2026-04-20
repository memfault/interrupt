#!/bin/bash
# Generate Matter ZAP artifacts from composable fragments.
#
# Usage: zap_generate.sh [output_dir]
#   output_dir defaults to src/default_zap
#
# Pipeline:
#   zap/base.json + zap/ma_dimmablelight.json
#       → output_dir/matter_echo.zap          (composed ZAP)
#       → output_dir/zap-generated/*.cpp/h     (C++ cluster code)
#       → output_dir/matter_echo.matter        (Matter IDL)
#
# Requires: zap-cli in PATH
# Takes ~8 seconds.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
MATTER_ROOT="${MATTER_ROOT:-$HOME/ncs/main/modules/lib/matter}"

ZAP_DIR="${1:-$PROJECT_DIR/src/default_zap}"
ZAP_FILE="$ZAP_DIR/matter_echo.zap"
ZAP_GENERATED="$ZAP_DIR/zap-generated"
ZCL_JSON="$MATTER_ROOT/src/app/zap-templates/zcl/zcl.json"
APP_TEMPLATES="$MATTER_ROOT/src/app/zap-templates/app-templates.json"
IDL_TEMPLATES="$MATTER_ROOT/src/app/zap-templates/matter-idl-server.json"
GENERATE_PY="$MATTER_ROOT/scripts/tools/zap/generate.py"

export PATH="$HOME/.local/bin:$PATH"
if ! command -v zap-cli &>/dev/null; then
    echo "Error: zap-cli not found. Install from https://github.com/project-chip/zap/releases" >&2
    exit 1
fi

mkdir -p "$ZAP_DIR" "$ZAP_GENERATED"

echo "=== ZAP generation (output: $ZAP_DIR) ==="

python3 "$PROJECT_DIR/tools/zap_compose.py" \
    "$PROJECT_DIR/zap/base.json" \
    "$PROJECT_DIR/zap/ma_dimmablelight.json" \
    > "$ZAP_FILE"

python3 "$GENERATE_PY" \
    --no-prettify-output \
    --templates "$APP_TEMPLATES" \
    --zcl "$ZCL_JSON" \
    --output-dir "$ZAP_GENERATED" \
    "$ZAP_FILE" 2>&1 | grep -E "^[✍🕐]" || true

python3 "$GENERATE_PY" \
    --no-prettify-output \
    --templates "$IDL_TEMPLATES" \
    --zcl "$ZCL_JSON" \
    --output-dir "$ZAP_DIR" \
    "$ZAP_FILE" 2>&1 | grep -E "^[✍🕐]" || true

[ -f "$ZAP_DIR/Clusters.matter" ] && mv "$ZAP_DIR/Clusters.matter" "$ZAP_DIR/matter_echo.matter"

echo "=== Done ==="
