-- Kolam - build with xmake (https://xmake.io)
--
--   ./run.sh                      build and run the game (same as the two lines below)
--   ./run.sh --dev                developer mode: saving a file, or F5, rebuilds and
--                                 restarts the game right where you were
--   xmake                         build the game for this computer: build/<system>/kolam
--   xmake run kolam               run it
--   xmake run kolam --check-pages check every puzzle page in src/story.cpp is solvable
--   xmake f -m debug              debug builds: raylib logs on (see main.cpp), N solves a puzzle page
--   xmake f -m release            back to normal builds
--   ./release.sh                  finished games for Linux, Windows and Android (see release.sh)
--   ./release.sh ios              the iPhone game, an unsigned .ipa (on a Mac only: GitHub
--                                 builds it on one when asked, see .github/workflows/build.yml)
--
-- xmake downloads what it needs by itself the first time (raylib, and Zig for
-- Windows / portable Linux builds). Everything it makes goes into build/ and .xmake/.

set_project("kolam")
set_version("1.0")
set_languages("c++14")
add_rules("mode.debug", "mode.release")
set_defaultmode("release")

-- "xmake f --portable=y": a Linux build that runs on older Linux systems too.
option("portable")
    set_default(false)
    set_showmenu(true)
    set_description("Build Linux for older systems too (glibc 2.31+), with Zig")
option_end()

-- Windows and portable Linux are built with Zig, a C/C++ compiler that can build
-- for other systems. xmake downloads it, and uses it for raylib too.
local use_zig = is_plat("mingw") or has_config("portable")
if use_zig then
    add_requires("zig 0.15.2")
    set_toolchains("zigcc@zig")
    if is_mode("release") then
        -- Zig builds its C++ runtime at the optimisation level seen when linking;
        -- without this it builds a debug runtime that's ~6 MB bigger
        add_ldflags("-O2", {force = true})
    end
end

if has_config("portable") then
    add_requires("raylib-portable 6.2", {configs = {toolchains = "zigcc@zig"}})
elseif is_plat("iphoneos") then
    -- iPhone: raylib runs on SDL there (see the raylib-sdl package below)
    add_requires("libsdl2", {configs = {sdlmain = true}})
    add_requires("raylib-sdl 6.2")
elseif use_zig then
    add_requires("raylib 6.2", {configs = {toolchains = "zigcc@zig"}})
else
    add_requires("raylib 6.2")
end

-- raylib for portable Linux. xmake's normal raylib package needs the X11 libraries
-- when it's built; this one only needs X11's headers, because raylib loads X11 when
-- the game starts. That keeps the game working on any Linux desktop.
package("raylib-portable")
    add_urls("https://github.com/raysan5/raylib/archive/refs/tags/$(version).tar.gz")
    add_versions("6.2", "df7a13129b1bec9e73a6962efff170beba310ee6eab8f94cd000b7316b4db5ee")
    add_syslinks("m", "pthread", "dl", "rt")
    on_install("linux", function (package)
        -- raylib needs X11's headers, but not the rest of this computer's /usr/include
                if not os.isdir("/usr/include/X11") then
            raise("X11 headers not found: install libx11-dev (Debian/Ubuntu) or libX11-devel (Fedora)")
        end
        os.mkdir("x11-headers")
        os.ln("/usr/include/X11", "x11-headers/X11")
        io.writefile("xmake.lua", [[
            add_rules("mode.debug", "mode.release")  -- so raylib gets optimised
            target("raylib")
                set_kind("static")
                set_languages("gnu99")
                set_warnings("none")
                add_files("src/*.c")
                add_includedirs("src/external/glfw/include")
                add_sysincludedirs("x11-headers")
                -- clipboard images are the one raylib feature that would need X11 at build time
                add_defines("PLATFORM_DESKTOP", "GRAPHICS_API_OPENGL_33", "_GLFW_X11", "SUPPORT_CLIPBOARD_IMAGE=0")
                add_headerfiles("src/raylib.h", "src/raymath.h", "src/rlgl.h")
        ]])
        import("package.tools.xmake").install(package)
    end)
package_end()

-- raylib for iPhone. raylib has no iOS layer of its own, but it can run on
-- SDL's (raylib's PLATFORM_DESKTOP_SDL layer, drawing with OpenGL ES 2), and
-- SDL runs on iOS. Only built on a Mac (Xcode): see release.sh and README.
package("raylib-sdl")
    add_urls("https://github.com/raysan5/raylib/archive/refs/tags/$(version).tar.gz")
    add_versions("6.2", "df7a13129b1bec9e73a6962efff170beba310ee6eab8f94cd000b7316b4db5ee")
    add_deps("libsdl2", {configs = {sdlmain = true}})
    add_frameworks("OpenGLES", "UIKit", "QuartzCore", "CoreGraphics", "CoreAudio",
                   "AudioToolbox", "AVFoundation", "Foundation")
    on_install("iphoneos", function (package)
        -- miniaudio (raylib's sound) needs Objective-C on iOS, for the audio
        -- session: so its file is built as Objective-C
        os.cp("src/raudio.c", "src/raudio.m")
        io.writefile("xmake.lua", [[
            add_rules("mode.debug", "mode.release")
            add_requires("libsdl2") -- (xmake hands it the same SDL as above)
            target("raylib")
                set_kind("static")
                set_languages("gnu99")
                add_packages("libsdl2")
                set_warnings("none")
                add_files("src/rcore.c", "src/rshapes.c", "src/rtextures.c",
                          "src/rtext.c", "src/rmodels.c", "src/raudio.m")
                add_includedirs("src")
                add_defines("PLATFORM_DESKTOP_SDL", "GRAPHICS_API_OPENGL_ES2")
                add_headerfiles("src/raylib.h", "src/raymath.h", "src/rlgl.h")
        ]])
        import("package.tools.xmake").install(package)
    end)
package_end()

-- Builds every file in assets/ into the game, so the game is one single file.
-- It writes a short list of the files (assets.cpp) that uses EMBED from
-- src/game.h; the files' bytes are copied in when that list compiles.
rule("embed_assets")
    on_config(function (target)
        local files = os.files(path.join(os.projectdir(), "assets", "**"))
        table.sort(files)
        local lines = {"// Generated by xmake.lua from assets/. Do not edit.", '#include "game.h"'}
        local table_lines = {"const EmbeddedFile EMBEDDED_FILES[] = {"}
        for i, file in ipairs(files) do
            local name = path.unix(path.relative(file, os.projectdir()))  -- e.g. assets/krita.gif
            -- the time stamp makes this list change (and rebuild) when an asset changes
            table.insert(lines, string.format('EMBED(file%d, "%s")  // %d', i, path.unix(file), os.mtime(file)))
            table.insert(table_lines, string.format('    {"%s", file%d_start, (unsigned int)(file%d_end - file%d_start)},', name, i, i, i))
        end
        table.insert(table_lines, "    {nullptr, nullptr, 0}};")
        local text = table.concat(lines, "\n") .. "\n" .. table.concat(table_lines, "\n") .. "\n"

        local out = path.join(target:autogendir(), "assets.cpp")
        if not os.isfile(out) or io.readfile(out) ~= text then  -- only rewrite when something changed
            io.writefile(out, text)
        end
        target:add("files", out)
    end)
rule_end()

target("kolam")
    set_kind("binary")
    add_rules("embed_assets")
    add_files("src/*.cpp")
    add_includedirs("src")
    if has_config("portable") then
        add_packages("raylib-portable")
    elseif is_plat("iphoneos") then
        add_packages("raylib-sdl", "libsdl2")
    else
        add_packages("raylib")
    end
    set_warnings("all")
        set_targetdir("$(builddir)/$(plat)")
    set_rundir("$(projectdir)") -- "xmake run" starts it here, so --dev can see src/
    if is_mode("debug") then
        add_defines("_DEBUG")
    end

    if is_plat("mingw") then
        add_ldflags("-static", "-Wl,--subsystem,windows")  -- no DLLs to ship, no console window
    elseif is_plat("android") then
        -- The game becomes libmain.so inside build/android/main.apk, signed with
        -- android/debug.keystore. Keep that key: phones only accept an update to the
        -- app if it's signed with the same key. (For an app store, make a release key.)
        add_rules("android.native_app", {
            android_sdk_version = "34",
            android_manifest = "android/AndroidManifest.xml",
            android_res = "android/res",
            keystore = "android/debug.keystore",
            keystore_pass = "android",
            package_name = "com.kolam.game",
            native_app_glue = false,  -- raylib already has it
        })
        set_basename("main")
        add_defines("PLATFORM_ANDROID")
        on_load(function (target)  -- save.cpp needs the NDK's "glue" header
            target:add("includedirs", path.join(get_config("ndk"), "sources", "android", "native_app_glue"))
        end)
        -- --wrap=fopen: raylib swaps in its own fopen on Android; without it the phone
        -- can't load the game at all. --no-undefined: anything missing stops the build here.
        add_shflags("-static-libstdc++", "-Wl,-u,ANativeActivity_onCreate", "-Wl,--wrap=fopen", "-Wl,--no-undefined",
                    {force = true})
        add_syslinks("log", "android", "EGL", "GLESv2", "OpenSLES", "m", "dl")
    elseif is_plat("iphoneos") then
        -- The game becomes Kolam.app, from ios/Info.plist (name, landscape,
        -- fullscreen). release.sh zips it into an unsigned .ipa; a sideloading
        -- tool (Sideloadly, AltStore) signs it with an Apple ID to install it.
        add_rules("xcode.application")
        add_files("ios/Info.plist")
        add_defines("PLATFORM_IOS")
        set_values("xcode.bundle_identifier", "com.kolam.game")
    end
target_end()
