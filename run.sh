#!/bin/sh
# Builds the game (only what changed) and runs it.
#   ./run.sh                 run the game
#   ./run.sh --check-pages   check the puzzle pages instead
#   ./run.sh --dev           developer mode: save a file in src/ or assets/ (or
#                            press F5 in the game) and the game rebuilds and
#                            reopens right where you were. Close the window to stop.
set -e
cd "$(dirname "$0")"
# xmake refuses to run as root (e.g. in WSL) unless told it is allowed
[ "$(id -u)" = 0 ] && export XMAKE_ROOT=y

if [ "$1" != "--dev" ]; then
  xmake -y -q
  exec xmake run kolam "$@"
fi

RESTART=build/dev-restart # the game writes "resume" or "home" here before closing

# Waits until a file in src/, assets/ or xmake.lua is saved
wait_for_a_change() {
  touch build/dev-stamp
  echo "== fix it and save: the game will build and open again"
  while [ -z "$(find src assets xmake.lua -newer build/dev-stamp -print -quit)" ]; do
    sleep 1
  done
}

args="--dev"
while true; do
  rm -f "$RESTART"
  if ! xmake -y -q; then # the build failed: the errors are above
    wait_for_a_change
    continue
  fi
  if ! xmake run kolam $args; then # the game crashed
    wait_for_a_change
    args="--dev --resume"
    continue
  fi
  [ -f "$RESTART" ] || exit 0 # the window was closed: stop
  case "$(cat "$RESTART")" in
    resume) args="--dev --resume" ;; # carry on from the same scene
    *) args="--dev" ;;               # back to the home screen
  esac
  echo "== rebuilding..."
done
