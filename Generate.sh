#!/usr/bin/env bash
# Configures and builds with CMake (Ninja if available, otherwise Makefiles).
#   ./Generate.sh [Config]            -> configure + build into build-<config>/   (default Debug)
#   ./Generate.sh Release run         -> ... then run the test suite
#   ./Generate.sh Release install     -> ... then install the DLL package into ./dist (used by ../BattlefieldGame)
#   ./Generate.sh -h | --help         -> show this help
set -euo pipefail
cd "$(dirname "$0")"

case "${1:-}" in
  -h|--help|-\?)
    sed -n '2,/^set -/p' "$0" | sed '$d' | sed 's/^# \{0,1\}//'
    exit 0 ;;
esac

CONFIG="${1:-Debug}"
ACTION="${2:-}"
DIR="build-$(echo "$CONFIG" | tr '[:upper:]' '[:lower:]')"
GEN=()
command -v ninja >/dev/null 2>&1 && GEN=(-G Ninja)

cmake -S . -B "$DIR" "${GEN[@]}" -DCMAKE_BUILD_TYPE="$CONFIG"
cmake --build "$DIR" --parallel

[ "$ACTION" = "run" ] && "$DIR/bin/BattlefieldTestRunner" --quiet
[ "$ACTION" = "install" ] && cmake --install "$DIR" --prefix dist
exit 0
