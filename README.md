# கோலம் Kolam

![Kolam](art/cover.png "Kolam")

A cozy story and puzzle game. Anu, a designer who likes her life in Hyderabad, goes home for Pongal and the temple festival. Shopping with Amma, old streets, old friends and her own childhood drawings bring back the little girl she was.

**Growing up doesn't mean leaving every part of your childhood behind.**

- What is a kolam: https://en.wikipedia.org/wiki/Kolam
- The math behind kolams: https://web.archive.org/web/20240911071012/https://www.cmi.ac.in/gift/kolam/kola_pattern.htm

| | | |
|---|---|---|
| ![Home screen](art/screenshots/home.png) | ![Hyderabad, 11:47 pm](art/screenshots/busy.png) | ![Amma in the kitchen](art/screenshots/kitchen.png) |
| *Home screen* | *Hyderabad, late at night* | *Home: colour comes back with each memory* |
| ![Appa's kite](art/screenshots/kite.png) | ![Amma's notebook](art/screenshots/notebook.png) | ![Amma's kolam book](art/screenshots/puzzle.png) |
| *Flashback: the kite, in full colour* | *Amma's notebook, the end of Chapter 1* | *A page of Amma's kolam book* |

## Play it

- **From a release build** (made with [`./release.sh`](#a-single-file-game-for-players)):
  - **Linux:** run `kolam-linux-x86_64`, a single file with nothing to install. Make it runnable first: `chmod +x kolam-linux-x86_64`.
  - **Windows:** run `kolam-windows-x86_64.exe`, a single file. If SmartScreen warns about an unknown app, choose *More info → Run anyway*.
  - **Android:** copy `kolam-android.apk` to the phone and open it. Allow "install unknown apps" for your file manager when asked. Or, with a USB cable: `adb install kolam-android.apk`. Tap where you'd click, and use the phone's Back button to pause.
- **From the source code:** see [Build and run](#build-and-run), then `./run.sh`.

## The story

**Anu Raghavan, 27**, designs apps for a Hyderabad startup, and she's good at it and likes it. She isn't running from the city, and home isn't a perfect place where everything is better: both are ordinary parts of her life. She just hasn't been home in a long time, and Amma's calls tend to arrive when she is busy.

One night Amma leaves a voice message: Pongal is coming, the temple festival preparations have started, and the house is quiet. Anu takes the night bus home to her village on the **Bhavani river, between Mettupalayam and Sathyamangalam** in western Tamil Nadu: the blue Nilgiri hills on one side, coconut groves and Sathyamangalam's jasmine fields on the other.

Over the days before the festival she shops with **Amma**, walks the old streets, meets a childhood friend, and finds Amma's kolam notebook and her own unfinished childhood drawing. **The temple festival** is the main event: it sets the timeline, the places, the puzzles and the big moments. **The kolam** is the thread through all of it, and the final kolam means something because the player has spent the game learning what it stands for.

**Colour and memories.** Colour shows how memories make familiar details noticeable again; it isn't a happiness meter. The present day is a little muted at first (never grey or gloomy), and each memory Anu finds brings colour back. Flashbacks and childhood imagination are always in full colour, and festival scenes are colourful anyway: lights, flowers, decorations.

**Work is not the enemy.** Work calls can interrupt, and the player can answer or say *Not now*. Neither choice changes the story or the colour.

**Few words.** Sounds, objects, animation and play tell the story. Conversations are everyday ones between a mother and daughter: no speeches, no one explaining how they feel. Hard lines (like Anu's *"Hi, Amma."*) only come out while the player holds the button.

**Memories are playable** where possible: rather than watching a flashback, you become little Anu for a moment (the kite now; the bicycle and others later).

The story is told **in order**, one chapter at a time. Each chapter starts with a title card, and once reached it can be replayed from Home → Chapters.

### Chapter 1: Home (the demo)

Built first, as the demo and mock-up for playtesting: does the play carry the feeling? It runs **about 10 to 15 minutes**: Hyderabad lasts 20 seconds, the bus ride 25, the kite needs 7 seconds in the wind. Text types fast by default (Settings → Text speed). The timings are constants at the top of each scene (`BUSY_SECONDS`, `JOURNEY_SECONDS`, `KITE_GOAL_SECONDS` in `minigames.cpp`; `FIRST_CALL_SECONDS` in `explore_scene.cpp`).

| Step | What happens | How you play |
|---|---|---|
| **Title card** | *Chapter 1: Home. Hyderabad, eleven days before Pongal.* | It moves on by itself after 3.5 seconds. |
| **Hyderabad, 11:47 pm** | Anu at her desk, designing an app's screens. Messages from work (mostly nice ones) pile up, and Amma calls three times. | Work is clicking a card on the screen, which then wants to be somewhere else, again and again. Swipe messages away; each of Amma's calls leaves a small green missed-call mark. At midnight (or when you tap it), everything falls away except Amma's voice message. |
| **The voice message** | Amma's message plays on Anu's phone, its words under the waveform. | Listen. Then type the reply she always sends (each key or tap types more): *"busy ma, call u later"*. She deletes it. The bus ticket appears: **Book**. |
| **The night bus** | No words. The city turns to fields, night to dawn, past Mettupalayam and the Bhavani. | Drag across the window to wipe the fog. Kolam dots appear in the fog one by one, join into a kolam for a moment, and fade. Click the phone to turn it face down if you like. Click to get off at dawn. |
| **Home, by the Bhavani** | The front step, the hall, the kitchen and the terrace, and Amma. | Explore and click things. Memories flood the world with colour for a moment, then it settles a little brighter. Trace the pencil height marks on the doorframe (the last one is from when she was fifteen). Meeting Amma, *"Hi, Amma."* only comes out while you **hold**. Work calls: answer (a short, friendly call) or *Not now*. The kite reel ends the scene. |
| **The kite** | A memory: little Anu flying a kite with Appa on Pongal morning. | You are little Anu. Pull to climb, but too hard and it wobbles and dives; let go as a gust comes and it lifts. Keep it in the high wind for 7 seconds. |
| **The terrace, after** | Night. Back in the present, the reel in her hands. | A quiet moment, no words. |
| **Coffee for Amma** | Early morning, before Amma is up. No words. | Hold to pour the decoction, then the milk. Then stretch it: hold to pour from the tumbler into the davara, moving up and down. Too low doesn't mix, too high splashes, in between it froths. No score. |
| **Five in the morning** | Amma, at the door with the rice flour, can't bend down. Anu takes the flour. *"What?"* Anu nods at the doorstep. | |
| **Amma's list** | Amma's kolam notebook, open at the festival kolam, which draws itself. Not yet. Amma: *"Come tomorrow. We have a lot to buy."* Her shopping list; the last item has a little drawing of flowers. *Tomorrow.* | Click to go on. End of Chapter 1. |

The full doorstep kolam (free drawing with mirrors, Amma's pattern underneath, and its ending) has moved to Chapter 8. Its code (`DoorstepScene` in `scenes.cpp`, `Doorstep()` in `story.cpp`) is kept for it.

### Inspirations

Kolam is its own game, but it learns from these. It takes ideas, never their stories, characters or look:

| Game | What Kolam takes from it | Where |
|---|---|---|
| **Florence** | The way you play *is* the feeling. Wordless moments, and small actions that change as the person changes. | Work that never ends; typing the usual reply and deleting it; wiping the bus window; holding to get "Hi, Amma." out; tracing the height marks |
| **Venba** | Cooking as love and memory in a South Indian family, and a parent's way of doing things handed down | Filter coffee for Amma; Amma's kolam notebook; cooking pongal (Chapter 5) |
| **Fishbowl** | Growing up told through a phone and a home's everyday objects | The voice message on the phone; Amma's calls leaving marks; the calendar with "Anu?"; the names scratched on the vessels |
| **To the Moon** | Objects that hold memories, and an ending that lands quietly | Each memory blooms into full colour; the kite reel opens a memory; the kolam in the bus fog; the flower on the list |
| **Coffee Talk** | Slow, warm moments, and making a drink for someone as a way of listening | The coffee at dawn: a quiet scene with nothing to win |

### The whole game (planned)

The chapters follow the days before Pongal and the temple festival:

| Chapter | When | What happens | New play |
|---|---|---|---|
| **1. Home** | 11 days before | Hyderabad, the bus, the homecoming *(built)* | Work card, voice message, fog, exploring, kite, coffee |
| **2. The List** | 10 days before | Shopping with Amma at the market. The flower shop (*"You used to know exactly which flowers you wanted."*), the bangle stall (*"You used to take so long to choose."*). One item is still missing at the end. | Shopping list and bag; find the right stall; pick the right item; flowers; garland patterns; paying with coins and notes |
| **3. Familiar Streets** | 9 days before | The old streets, school and playground; a childhood friend. *"Do you still draw?" "Not really."* | Five stones, hopscotch, paper boats, finding pieces of a drawing, putting an old photo back together (all optional) |
| **4. Amma's Book** | 7 days before | Amma's kolam notebook: patterns, notes about little Anu, and the unfinished drawing: *"I will finish this when I am big."* | The kolam book puzzles (built), each followed by a short memory; the scrapbook |
| **5. Preparations** | 4 days before | Getting the house and the temple street ready, with what was bought in Chapter 2. *"Pongalo Pongal!"* | Flower decorations, garlands, mango-leaf thoranam, lamp symmetry, cooking pongal, a short entrance kolam |
| **6. Festival Night** | Festival eve | The festival with her friend: lights, stalls, a paper windmill. Amma: *"Still the same."* | Ring toss, a target game, building a paper windmill, matching patterns, finding a stall, festival tokens |
| **7. The Little Girl** | Festival eve, late | The notebook opens into a world of crayon drawings. Little Anu and Anu meet, and draw together. | Exploring the crayon world, finding colours, drawing together |
| **8. Five in the Morning** | Pongal morning | Anu draws the kolam, Amma guiding, little Anu beside her. A tiny flower in the corner. | The doorstep kolam (built): free drawing, mirrors, Amma's pattern underneath |
| **9. What We Carry** | After | The kolam on the step; then Hyderabad, work, and a flower drawn on a sticky note. | |

The build order is the same: Chapter 1 first, then shopping, the streets and scrapbook, the notebook and preparations, the festival, and the crayon world with the finale. No combat, lives, hard timers or platforming; shopping and festival games should be fun, never chores.

## The game

A cozy story game made with [raylib](https://www.raylib.com/) in C++.

- **Explore** Anu's home: the front step, the hall, the kitchen and the terrace. Click things to remember them. Each memory comes back as a *pulli* (a kolam dot) at the top of the screen, and the world gets more colourful.
- **The phone** rings while you explore: work. Answer for a short, friendly call, or *Not now*. Neither changes anything.
- **Flashbacks** you play as little Anu, like flying a kite with Appa on a Pongal morning nineteen years ago.
- **Amma's kolam book.** Each page is a *sikku kolam* puzzle: one line has to loop around every dot without touching it. Click between two dots to put up a wall, and the line bounces off it. Join every separate line into one, and it draws itself in a single stroke like a real kolam.
- **Little Anu's pages** in the same book are on school-notebook paper in crayon, and the last one is the kolam she promised to finish "when I am big".
- **The night bus**: no words, just the window, the fog, a kolam in the fog, and the phone.
- **Coffee for Amma**, and **Amma's list**, which ends Chapter 1.
- **The doorstep kolam** at dawn (built; it waits for Chapter 8).
- **Home screen** with Continue, New Game, Chapters, Settings and Exit. Progress, including the memories you've found, saves automatically.
- **Settings:** master volume, music volume, text speed, windowed or fullscreen, screen shape and screen edges.
- **Any screen size or shape:** the game is 16:9 by default. Settings → Screen shape switches to *Fill screen* (as wide as your window or phone), *4:3* or *Square*. It stays sharp at any size, from a small window to a 4K monitor or a phone. If the window's shape doesn't match, the space beside the game is filled with a soft glow of the scene or with black bars (Settings → Screen edges).

## How to play

| Where | Controls |
|---|---|
| Menus | Mouse, or **Up/Down** and **Enter**. **Left/Right** changes a setting. **Esc** goes back. |
| Story | **Click**, **Space** or **Enter**: show the whole line, then go to the next one |
| Hyderabad | **Click** the card on the screen to work and **click** messages to clear them. Or tap Amma's voice message and stop. |
| Voice message | **Any key or click** types the reply. **Click** *Book*. |
| Hard lines | When a ring pulses in the dialogue box, **hold** the mouse button (or Space) to say the line. |
| Coffee | **Hold** to pour. When stretching, **move the mouse up and down** to pour from higher or lower, and **let go** to pour it back. |
| Night bus | **Drag** across the window to wipe the fog. **Click** the phone to turn it over. At dawn, **click** to get off. |
| Exploring | **Click** things to look at them. The tabs at the bottom change rooms; a dot on a tab means something there is waiting for you. On the doorframe, **drag up** to trace the height marks. |
| The kite | **Hold** the mouse button (or **Space** / **Up**) to pull, **let go** to give slack, **move** the mouse (or **Left** / **Right**) to steer. Let go as a gust comes to lift it. Keep it in the strong wind. |
| Amma's list | **Click** to go on. |
| Kolam book | **Click** between two dots to add or remove a wall. **R** starts the page over. |
| Doorstep | **Drag** to draw (don't touch the dots). **H** / **V** toggle the mirrors, **Z** undoes, **C** clears, **Enter** finishes. |
| Anywhere | **Esc** pauses (Resume, Save game, Settings, Main menu). **F11** toggles fullscreen. |

On **Android**, tap where you'd click and hold where you'd hold. The **Back** button pauses. A few keyboard-only extras don't have a touch button yet: **R** to restart a kolam page, and **H/V** to toggle the doorstep mirrors (both mirrors are on by default).

## Build and run

The game is built with [xmake](https://xmake.io) (Arch: `pacman -S xmake`; others: see [installing xmake](https://xmake.io/#/guide/installation)) and a C++ compiler. xmake gets [raylib](https://www.raylib.com/) by itself: it uses the one installed on your system if there is one, otherwise it downloads and builds it on the first run.

```bash
./run.sh                          # build (only what changed) and run the game
./run.sh --dev                    # developer mode: rebuilds and reopens when you save a file (see below)
xmake                             # just build -> build/linux/kolam (or build/<your system>/)
xmake run kolam                   # run it
xmake run kolam --check-pages     # check that every puzzle page is solvable
xmake f -m debug                  # switch to debug builds: raylib logs on, N solves a puzzle page
xmake f -m release                # back to normal builds
xmake clean                       # delete what was built
```
All the build settings are in `xmake.lua`, in one short file with comments.

#### Developer mode: see changes as you save

```bash
./run.sh --dev
```
The game opens as usual. Then, whenever you **save a file in `src/` or `assets/`** (code, story, art), or press **F5** in the game:
1. the game saves and closes,
2. only what changed is rebuilt (a few seconds),
3. it opens again **right where you were**: the same scene, with the memories you'd found. The intro is skipped.

If the code doesn't compile (or the game crashes), the errors stay in the terminal; fix them and save, and it carries on. Close the game window to stop. A small "DEV" note in the corner shows when developer mode is on; players never see it.

This restarts the game rather than swapping code while it runs. Swapping C++ code in a running game is possible, but fragile and complicated, while a restart into the same scene is quick and always reliable.

### A single-file game for players

```bash
./release.sh                   # every platform listed in PLATFORMS at the top of release.sh
./release.sh android           # just Android
./release.sh linux windows     # just these two
```
To change what a plain `./release.sh` builds, edit the `PLATFORMS="linux windows android"` line at the top of `release.sh`.

| Platform | You get (in `build/release/`) | Runs on |
|---|---|---|
| `linux` | `kolam-linux-x86_64`, one single file | Most Linux systems: glibc 2.31 or newer (Ubuntu 20.04+, Debian 11+, Fedora, Arch...) |
| `windows` | `kolam-windows-x86_64.exe`, one single file | Windows 10 and 11, with no DLLs or installer |
| `android` | `kolam-android.apk` | Android 7.0 and newer, 64-bit ARM phones (nearly all phones since 2017) |

Everything (fonts, music, images) is inside each of them. Saves and settings go in the player's home folder (on Android, the app's private storage).

**How it works:** `release.sh` is only xmake commands, one build per platform. Everything else is in `xmake.lua`:
- **Linux and Windows** are built with [Zig](https://ziglang.org/), a C/C++ compiler that can build for other systems. xmake downloads it, and builds raylib with it.
- **Linux** uses `--portable=y`, which targets an older glibc so the game runs on older Linux too.
- **Android** uses xmake's `android.native_app` rule to make and sign the APK.
- **Each platform keeps its own xmake settings**, so a release never changes your everyday `xmake` setup.

**What you need:** `xmake`, and the X11 headers for Linux (already on most Linux desktops). For Android you also need the Android SDK and NDK, set up once as shown below. No sudo, no Docker.

**How long it takes:** the first release downloads Zig and builds raylib for each platform, which takes a few minutes. After that, a release takes seconds, because only the game itself is recompiled.

#### One-time Android setup

`release.sh` looks for Android's SDK in `~/Android/Sdk`, where Android Studio puts it (or set `ANDROID_HOME`). If you don't have Android Studio, install just the parts the game needs (about 1.5 GB). This needs Java (JDK 17 or newer) and asks you to accept Google's licenses:
```bash
curl -LO https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip
mkdir -p ~/Android/Sdk/cmdline-tools && unzip -q commandlinetools-linux-*_latest.zip -d ~/Android/Sdk/cmdline-tools
mv ~/Android/Sdk/cmdline-tools/cmdline-tools ~/Android/Sdk/cmdline-tools/latest
~/Android/Sdk/cmdline-tools/latest/bin/sdkmanager --licenses
~/Android/Sdk/cmdline-tools/latest/bin/sdkmanager "ndk;27.2.12479018" "build-tools;34.0.0" "platforms;android-34"
```

**Android files** are in `android/`:
- `AndroidManifest.xml` sets the app's name, icon, screen orientation and package name (`com.kolam.game`).
- `res/drawable/icon.png` is the app icon.
- `debug.keystore` is the key the APK is signed with (password `android`). **Keep it:** phones only accept an update to the app if it's signed with the same key. For the Play Store you'd make a proper release key instead.

Every push is built on GitHub (`.github/workflows/build.yml`), which runs `./release.sh linux`, checks the puzzle pages, and attaches the Linux game to the run.

## Working on the game

```
kolam/
├── xmake.lua     how to build (see above)
├── run.sh        builds and runs the game
├── release.sh    builds the finished games for Linux, Windows and Android
├── src/          the code: start with game.h
├── assets/       fonts, music, sounds, the intro gif: built into the game when it compiles
├── android/      the Android app's manifest, icon and signing key
├── art/          design sources (posters, Krita files, screenshots); not used by the game
└── build/        made by xmake, never edit or commit (xmake also keeps notes in .xmake/)
```

All the code is in `src/`. Start with `game.h`: it declares everything and says which file does what.

| File | What it does |
|---|---|
| `story.cpp` | **All the content**: dialogue, puzzle pages and the order they come in. Writers only need this file. |
| `main.cpp` | Window, main loop, every screen (home, settings, pause, playing), and the screen shader that turns the world grey or colourful |
| `art.cpp` | The people (Amma, Anu, little Anu, Appa) and shared scenery (hills, palms, jasmine, brass pots, tiled floors, soft shadows), drawn with shapes until the real art arrives |
| `scenes.cpp` | Intro animation, chapter title cards, dialogue scenes, Amma's list (the end of Chapter 1), and the doorstep drawing (for Chapter 8) |
| `explore_scene.cpp` | Walking around the house: the rooms (drawn with shapes for now), what's clickable where, memories, the doorframe close-up, the phone |
| `minigames.cpp` | Hyderabad, the voice message, the night bus, the kite, and filter coffee |
| `puzzle_scene.cpp` | A page of the kolam book |
| `kolam.cpp` | The puzzle rules and how kolam lines are drawn |
| `ui.cpp` | Fonts, menu buttons, text helpers, the home screen animation |
| `audio.cpp` | Sounds: each place's background sound (fading from one to the next), effects, and loops like pouring |
| `save.cpp` | Settings, save-game files, and the memories Anu has found (`Progress`) |
| `game.h` | Shared declarations and colors |

### Drawing art for the game

The game's look is meant to be **hand-drawn**: each story moment is an illustrated scene with one small interaction. Until a scene's art exists, it's drawn with shapes in code, so the game is always playable. The stand-in art is made to look drawn too:
- **`art.cpp`** has the people, shaded with the light from the upper left, and scenery pieces the places share. Use them rather than plain rectangles: `DrawPalm()`, `DrawHills()`, `DrawJasmine()`, `DrawPot()`, `DrawTiledFloor()`, `DrawSoftShadow()`.
- **The setting:** a village on the Bhavani between Mettupalayam and Sathyamangalam. That means blue Nilgiri hills, coconut groves, jasmine, a lime-washed house with a clay-tiled roof and a thinnai, and a red oxide floor inside.

When you draw a scene:
- **Canvas:** 1920×1080 pixels (16:9). That's 1.5 pixels per game unit, so the stage, the centred 720×720 square, is the middle 1080×1080 pixels. Keep anything important (characters, things to click) inside the middle 1440×1080 (4:3); the outer edges can be cropped when the screen shape is 4:3 or square.
- **Layers:** export each layer as its own PNG **at the full 1920×1080 size**, transparent everywhere except what's drawn on it. Each layer then lands exactly where you drew it, so the game needs no position numbers. Typical layers: `background`, the people (`amma`), each thing that can be clicked (`grinding_stone`), anything that moves (`steam`), and a `foreground`.
- **Where:** `assets/scenes/<scene name>/<layer name>.png`, e.g. `assets/scenes/kitchen/amma.png`. Everything in `assets/` is built into the game automatically.
- **Krita:** keep the `.kra` source files in `art/`, and export PNGs from there.

The code to show these layers comes next, as the first scenes are drawn.

### Sounds

Every sound is an `.ogg` file in `assets/sounds/`, used by its name (without `.ogg`). Three calls, all in `audio.cpp`:
```cpp
PlayAmbience("crickets", 0.5f);   // the place's background sound, looping; the old one fades out
PlayEffect("click");               // once
PlayLoop("pour");                  // keeps going while it's called every frame
```
Scenes call `PlayAmbience()` in their `Update()`, so a place always has its sound, even after loading a save. Menus are quiet, and the song plays softer while a place has its own sound. Every button already clicks (`Clicked()` in `ui.cpp`).

| Sound | Where |
|---|---|
| `city_night` | Hyderabad and the voice message |
| `bus` | The night bus |
| `birds_day` | Home by day (softer indoors); daytime scenes |
| `crowd` | The terrace: the festival far off |
| `crickets` | Night scenes |
| `morning` | Coffee, Amma's list, the doorstep |
| `wind`, `gust` | The kite (letting go into a gust whooshes) |
| `notify`, `vibrate` | Work messages; Amma's calls and work calls |
| `click`, `page`, `scribble`, `pencil` | Buttons and typing; the notebook and kolam book; walls in the kolam book; tracing and drawing |
| `pour` | Pouring coffee (louder from higher) |
| `bell`, `temple_bells` | A memory coming back; the temple tower |

To add one: find a sound you're allowed to use (CC0 is easiest), make it a small mono `.ogg` (e.g. `ffmpeg -i in.wav -ac 1 -c:a libvorbis -q:a 2 assets/sounds/name.ogg`), and add it to [Credits](#credits).

### Add or change dialogue

The story is `Chapters()` at the bottom of `story.cpp`: a list of chapters, each a list of steps played in order. An empty speaker means narration:
```cpp
{"home", "Chapter 1", "Home", "Hyderabad, eleven days before Pongal", {   // id, heading, title, where
    Busy(),
    VoiceMessage("Amma's voice message", {   // played on Anu's phone
        {"Amma", "Kanna. It's Amma."},
        {"Amma", "Pongal is on the fourteenth."},
    }),
    Travel(),
    Talk("Home", {
        {"Amma", "Anu..."},
        {"Anu", "Hi, Amma.", 2.5f},   // hard to say: hold the button 2.5 s for the words to come out
    }),
    Talk("Five in the morning", {
        {"Amma", "What?"},
    }).At("front_step", NIGHT),   // shown over a place: front_step, hall, kitchen, terrace
    Talk("The terrace, after", {}).At("terrace", NIGHT),   // no words: a quiet moment there
    Memory("...", {...}),      // a flashback: same as Talk, but in full colour
    Puzzle(2),                 // page 3 of the kolam book
    AmmasList(3, {{"Amma", "Come tomorrow."}, {"", "Banana leaves"}, {"", "Flowers"}}),
                               // her notebook at page 4's kolam, what she says, then the list
    Kite(),  Coffee(),  Doorstep(),   // the other minigames
}},
```
- **Keep it short.** If the picture can say it, cut the line.
- **A new chapter** is one more `{id, heading, title, where, {steps}}` in the list. The `id` is never shown, but saves and the Chapters menu remember chapters by it, so don't change it once players have saves.
- The save game remembers the chapter and the step number in it, so **add steps at the end of a chapter rather than in the middle**. Otherwise existing saves resume in the wrong place.
- Reached chapters are kept in `chapters.txt` (next to the saves), with the memories Anu had found by then, so replaying one starts with the right amount of colour.

### Add things to explore

An `Explore` step lists what can be clicked, and what happens when it is:
```cpp
Explore("Home", false /* night? */, {
    {"swing", "The swing", {                 // object, the name shown on hover, then the lines
        {"", "It creaks the same way."},
    }, "The swing"},                         // the memory Anu gets back (brings colour back)
    {"kite_reel", "Appa's kite reel", {...}, "", true},   // true = clicking this moves the story on
}),
```
*Where* each object sits is in `ROOM_OBJECTS` at the top of `explore_scene.cpp`, with each room's drawing just below it. To add a new object, give it a name and a rectangle there, draw it in its room, then use the name in `story.cpp`.

### Add a puzzle page

Pages are in `Pages()` in `story.cpp`. A page is drawn as text, one character per point:

```
"+.+.+.+",      o  a dot            +  a corner (ignored)
".o#o.o.",      #  wall in ink      .  gap in ink         (the player can't change these)
"+.+.+?+",      ?  a gap the player can turn into a wall
".o#o#o.",      *  a wall the player can turn into a gap
"+.+.+.+",
".o.o.o.",      The outer border is ignored and always acts as a wall.
"+.+.+.+",
```

The kolam line runs diagonally between the dots. It crosses straight through gaps and bounces off walls. A page is solved when it becomes **one** line. Rules of thumb:
- Lock most of the page in ink (`#` and `.`) and leave a few `?` spots for the player.
- A symmetric page (every change is mirrored to all four corners) needs an **odd** number of dots across and down, because an even-sized symmetric kolam can never be one line.

Then check every page is solvable and doesn't start already solved:
```bash
./run.sh --check-pages
```

### Good to know

- **The stage and the screen:**
  - The screen is 720 units tall and as wide as the chosen shape: 1280 for 16:9, 960 for 4:3, 720 for square.
  - Everything is positioned on the **stage**, the 720×720 square in the middle, from (0, 0) to (720, 720), so menus, dialogue and puzzles stay centred on any shape.
  - `FullScreen()` is the whole screen in the same units (its left edge is negative on wide screens). Use it, or `DrawBand()` / `DrawBandGradient()`, for backgrounds, so they reach the edges. Anything that belongs in a screen corner (like the memory dots) uses `FullScreen().x`.
  - The mouse always reports stage units, so scenes never need to know the window's size.
  - `main.cpp` draws each frame at the window's real resolution (a `Camera2D` zooms the units up), and the fonts are made to match (`LoadFonts()` in `ui.cpp`). Nothing is stretched after drawing, which keeps text sharp.
- **Esc belongs to the pause menu:** scenes shouldn't use it.
- **New `.cpp` files** in `src/` are picked up by xmake automatically.
- **New assets** go in `assets/`. xmake builds every file there into the game (the `embed_assets` rule in `xmake.lua`), so the finished game is one single file. Load them from memory with `FindAsset("assets/...")` and raylib's `Load...FromMemory` functions (see `LoadFonts()` in `ui.cpp`).
- **Colour:** every scene has a `colour` from 0 (grey) to 1 (full colour), and `main.cpp` runs the whole screen through a small shader that mixes in grey to match. Present-day scenes use `WorldColour()` (in `save.cpp`), which starts a little muted and rises with each memory found (`AddMemory()`). Work calls don't change it. Flashbacks use 1.
- **New kind of scene:** subclass `Scene` (in `game.h`). Load things in the constructor, free them in the destructor, set `done = true` when it's finished, and create it in `MakeScene()` in `scenes.cpp`.
- **Shared helpers** for scenes are in `ui.cpp`: `ContinuePressed()` / `BackPressed()` for input, `Clicked()` / `DrawButton()` for buttons, `AdvanceDialogue()` / `DrawDialogueBox()` for dialogue, and `Between()` / `Faded()` for fading things in and out. Use them rather than writing your own, so every scene behaves the same.
- **Formatting:** run `clang-format -i src/*.cpp src/*.h` before committing.
- **Where saves and settings go:**
  - Linux: `~/.local/share/kolam/`
  - Windows: `%APPDATA%/Kolam/`
  - macOS: `~/Library/Application Support/Kolam/`
  - Android: the app's private storage.

## Credits

- Akielsh - Developed
- Nithis - Developed
- Fonts: [Baloo Thambi 2](https://fonts.google.com/specimen/Baloo+Thambi+2) by Ek Type and [Kalam](https://fonts.google.com/specimen/Kalam) by Indian Type Foundry, both under the SIL Open Font License (see `assets/fonts/`)
- Made with [raylib](https://www.raylib.com/)
- Sounds, all [CC0](https://creativecommons.org/publicdomain/zero/1.0/) from [Freesound](https://freesound.org), trimmed and made quieter or louder to fit:
  - `city_night`: [DELHI ATMO busy city at night from far distance](https://freesound.org/s/509932/) by SoundEnsemble
  - `bus`: [bus_interior_highway](https://freesound.org/s/264409/) by ivolipa
  - `birds_day`: [North India Countryside Birds chirping](https://freesound.org/s/690626/) by Audiobaaz911
  - `morning`: [Morning In Palyem (North Goa, India)](https://freesound.org/s/272176/) by venaportae
  - `crickets`: [Night cricket ambience](https://freesound.org/s/130412/) by Abinash87
  - `crowd`: [India outdoor crowd](https://freesound.org/s/423510/) by Martin.Sadoux
  - `temple_bells`: [Indian Temple Bell](https://freesound.org/s/466652/) by ganiket
  - `bell`: [temple_bell_002](https://freesound.org/s/668647/) by tec_studio
  - `wind`: [breeze](https://freesound.org/s/181801/) by keweldog
  - `gust`: [Wind Gust](https://freesound.org/s/146932/) by crashoverride6
  - `notify`: [Notification Sound](https://freesound.org/s/235911/) by yfjesse
  - `vibrate`: [Phone vibration](https://freesound.org/s/515295/) by Breviceps
  - `click`: [Normal click](https://freesound.org/s/448086/) by Breviceps
  - `pour`: [Serving Water Slowly](https://freesound.org/s/241880/) by rafael45
  - `pencil`, `scribble`: [Drawing with a pen](https://freesound.org/s/407405/) by florianreichelt
  - `page`: [Page Turn](https://freesound.org/s/136778/) by davidbain

### Links

- YouTube Gameplay: _coming soon_
- itch.io Release: _coming soon_

## License

This project's sources are licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. See [LICENSE](LICENSE) for details. The fonts keep their own SIL Open Font License.
