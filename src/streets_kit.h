// streets_kit.h - drawing and talking helpers shared by Chapter 3 "Familiar
// Streets" (ch3_streets.cpp) and Chapter 6 "Festival Night"
// (ch6_festival.cpp): the village street drawn as a Tamil comic, comic speech
// balloons, children and other village folk, lamps, paper windmills and the
// temple tower. Everything is in `namespace streets`, so it can't clash with
// other files. Defined in streets_kit.cpp.
#pragma once

#include <string>
#include <vector>

#include "game.h"

namespace streets {

// People's skin and hair (the same as art.cpp's), and their outline ink
const Color SKIN = {184, 124, 86, 255};
const Color SKIN_DARK = {150, 96, 68, 255};
const Color HAIR = {48, 40, 62, 255};
const Color PERSON_INK = {58, 34, 40, 255};
// The printer's colours for the street
const Color RED_OXIDE = {204, 64, 44, 255};
const Color TEAK = {164, 94, 46, 255};
const Color TEAK_DARK = {112, 58, 30, 255};
const Color MARIGOLD = {250, 150, 30, 255};
const Color LEAF = {58, 152, 74, 255};
const Color LEAF_DARK = {36, 110, 56, 255};
const Color EARTH = {196, 154, 108, 255};
const Color BULB = {255, 222, 120, 255};

Vector2 Lerp2(Vector2 a, Vector2 b, float t);
float Approach(float value, float target, float rate); // eases, per frame

// The step's line `i`, or `fallback` if story.cpp gave fewer lines
Line LineAt(const std::vector<Line> &lines, size_t i, const Line &fallback);

//--- Words, in comic balloons --------------------------------------------------
// A balloon pointing down at `mouth`, the line typed up to `typed` letters.
// A hard line (hold > 0) shows a ring that fills while it's held.
void DrawBalloon(const Line &line, float typed, Vector2 mouth, float time);

// A few lines, said one after another; a click (or Space) moves them on,
// as everywhere else in the game (it uses AdvanceDialogue).
struct Balloons {
  std::vector<Line> lines;
  size_t current = 0;
  float typed = 0;
  bool active = false;
  void Start(const std::vector<Line> &say);
  bool Update(); // true on the frame the last line is dismissed
  // `anu` is where Anu's (or little Anu's) mouth is; `other` everyone else's
  void Draw(Vector2 anu, Vector2 other, float time) const;
};

// A line said in passing: it shows for a few seconds by itself, no click
struct Quip {
  Line line;
  float shown = 0; // seconds left
  float typed = 0;
  void Say(const Line &said, float seconds = 3.2f);
  void Update();
  void Draw(Vector2 mouth, float time) const;
  bool Showing() const { return shown > 0; }
};

// "Tap here": a soft pulsing ring, like the rest of the game's hints
void DrawHint(Vector2 at, float time, float alpha = 1);
// "Go on": the game's quiet ring in the corner
void DrawGoOn(float time);
// A fade from or to dark: 1 = black
void DrawCurtain(float amount, Color colour = BLACK);

//--- People --------------------------------------------------------------------
// Children and other village people, drawn in the same spirit as art.cpp's
// (warm outlines, big eyes with catch-lights), only simpler.
struct Folk {
  float height = 190;
  float head = 0.135f; // head radius, a fraction of height (children: big)
  Color top = {24, 110, 116, 255};    // blouse or shirt
  Color bottom = {236, 128, 48, 255}; // pavadai, saree, veshti or shorts
  Color border = {24, 128, 130, 255}; // the hem's border
  enum Hair { PLAITS, SHORT, BUN, BALD } hair = PLAITS;
  bool skirt = true;     // a pavadai or saree to the feet (or shorts)
  bool jasmine = false;  // flowers in her hair
  bool moustache = false;
  bool grey = false;
};
// `hand`: where the right hand is, in fractions of height from the feet, as
// DrawAnu's. `look`: -1..1. `happy`: laughing, eyes shut in a smile.
void DrawFolk(const Folk &f, float x, float feetY, float time,
              Vector2 hand = {0.17f, 0.42f}, float look = 0,
              bool happy = false);
// Little Meena, eight, in an orange pavadai; and Meena's boy, two or so
Folk LittleMeena(float height);
Folk MeenasBoy(float height);

//--- The village street --------------------------------------------------------
// A comic sky (day: blue with dots and clouds; night: deep blue and stars)
void DrawStreetSky(float bottom, bool night, float time, float scroll);
// A row of hills and palms far off, sliding by at `scroll`
void DrawFarVillage(float groundY, bool night, float time, float scroll);
// One house of the street: lime-washed wall, clay-tile roof, door, window,
// thinnai; `style` picks its colours and details
// For things repeated along a scrolling street, drawn from x = left - spacing
// - fmod(moved, spacing) every `spacing`: the place number of the first one.
// Add 1 for each one after it. (Working the number out from x instead
// flickers: a hair either side of a whole number gives a different thing.)
int StablePlace(float moved, float spacing, float left);
void DrawStreetHouse(float x, float groundY, float width, float height,
                     int style, float time, bool night);
// The packed-earth street from `y` down, scrolled by `scroll`
void DrawStreetGround(float y, float scroll, bool wet);
// A doorstep kolam on the ground
void DrawGroundKolam(Vector2 c, float size, int style, float alpha = 1);
// The temple tower (gopuram): `size` 1 is about 200 tall; `lights` 0..1 how
// many of its festival bulbs are on
void DrawTempleTower(float x, float baseY, float size, float lights,
                     float time);
// A string of festival bulbs hanging from a to b
void DrawBulbString(Vector2 a, Vector2 b, float sag, float time, int seed);
// Paper flags (bunting) on a string
void DrawBunting(Vector2 a, Vector2 b, float sag, float time);
// People far off at the festival: heads and shoulders, printed in dots
void DrawCrowd(float y, float scroll, float time, bool night);

//--- Things --------------------------------------------------------------------
// A clay lamp (agal); `lit` 0..1. Its glow is drawn separately, on top of
// any night shading, with DrawAgalGlow.
void DrawAgal(Vector2 c, float scale, float lit, float time);
void DrawAgalGlow(Vector2 c, float scale, float lit, float time);
// A paper windmill seen from the front, turned by `angle` (radians)
void DrawWindmill(Vector2 c, float r, float angle, int colours);
void DrawWindmillStick(Vector2 c, float r, Vector2 hand);
// A pebble
void DrawPebble(Vector2 c, float r, int seed);
// Rain falling over the whole screen
void DrawRain(float time, float amount);
// Sparkles of colour for a moment of joy
void DrawConfetti(Vector2 c, float age, int seed);

} // namespace streets
