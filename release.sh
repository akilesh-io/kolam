#!/bin/sh
# Builds the finished game for players with xmake (see xmake.lua), into build/release/:
#   linux     kolam-linux-x86_64          one file, for Linux with glibc 2.31 or newer
#   windows   kolam-windows-x86_64.exe    one file, for Windows 10/11
#   android   kolam-android.apk           for Android 7.0+, 64-bit ARM phones
#   ios       kolam-ios.ipa               for iPhone/iPad, iOS 13+: unsigned. Only on a
#                                         Mac with Xcode (GitHub builds it on one: see
#                                         .github/workflows/build.yml). To install it,
#                                         a sideloading tool signs it with an Apple ID.
#
#   ./release.sh                  every platform in PLATFORMS
#   ./release.sh android          just Android
#   ./release.sh linux windows    just those two
#   ./release.sh ios              the iPhone game (on a Mac)
PLATFORMS="linux windows android"

# Android needs Google's Android SDK + NDK (see README). Change this if yours is elsewhere.
ANDROID_SDK="${ANDROID_HOME:-$HOME/Android/Sdk}"

set -e
cd "$(dirname "$0")"
# xmake refuses to run as root (e.g. in WSL) unless told it is allowed
[ "$(id -u)" = 0 ] && export XMAKE_ROOT=y
[ $# -gt 0 ] && PLATFORMS="$*"
mkdir -p build/release

# build NAME [xmake options]: build with xmake. Each platform keeps its own
# xmake settings, so a release never changes your everyday "xmake" setup.
build() {
  name=$1; shift
  echo "== building for $name"
  export XMAKE_CONFIGDIR="$PWD/build/xmake-config/$name"
  # -c: check the compilers afresh, so a check saved by an earlier release never
  # points at a compiler that has since moved (downloaded packages are kept)
  xmake f -c -y -m release -o build/release-work "$@"
  xmake -y
}

for platform in $PLATFORMS; do
  case $platform in
    linux)
      build linux -p linux -a x86_64 --portable=y --cross=x86_64-linux-gnu.2.31
      cp build/release-work/linux/kolam build/release/kolam-linux-x86_64 ;;
    windows)
      build windows -p mingw -a x86_64
      cp build/release-work/mingw/kolam.exe build/release/kolam-windows-x86_64.exe ;;
    android)
      NDK=$(ls -d "$ANDROID_SDK"/ndk/* 2>/dev/null | sort -V | tail -1)  # the newest NDK
      [ -n "$NDK" ] || { echo "No Android NDK in $ANDROID_SDK/ndk (see README)"; exit 1; }
      build android -p android -a arm64-v8a --ndk_sdkver=24 --android_sdk="$ANDROID_SDK" --ndk="$NDK"
      cp build/release-work/android/main.apk build/release/kolam-android.apk ;;
    ios)
      [ "$(uname)" = Darwin ] || { echo "The iPhone game can only be built on a Mac with Xcode (see README)"; exit 1; }
      # unsigned (an ad-hoc signature): a sideloading tool signs it to install it
      build ios -p iphoneos -a arm64 --target_minver=13.0 --xcode_codesign_identity=n
      app=$(find build/release-work/iphoneos -maxdepth 4 -type d -name "*.app" | head -1)
      [ -n "$app" ] || { echo "No .app was built"; exit 1; }
      # an .ipa is the .app in a Payload folder, zipped
      rm -rf build/ipa && mkdir -p build/ipa/Payload
      cp -R "$app" build/ipa/Payload/
      rm -f build/release/kolam-ios.ipa
      (cd build/ipa && zip -qry ../release/kolam-ios.ipa Payload) ;;
    *)
      echo "Unknown platform '$platform'. Use: linux, windows, android, ios"; exit 1 ;;
  esac
done
ls -lh build/release/
